#include "Image.h"
#include "GlobalDeviceObjects.h"
#include "DeviceMemoryManager.h"
#include "SwapChain.h"
#include "StagingBuffer.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Queue.h"
#include "VulkanUtil.h"

Image::~Image()
{
	vkDestroyImageView(GetDevice()->GetDeviceHandle(), m_view, nullptr);
	if (m_shouldDestoryRawImage)
		vkDestroyImage(GetDevice()->GetDeviceHandle(), m_image, nullptr);
}

bool Image::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, const VkImageCreateInfo& info, uint32_t memoryPropertyFlag)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	// Check if physical device supports input format along with its tiling feature
	VkFormatProperties formatProp = pDevice->GetPhysicalDevice()->GetPhysicalDeviceFormatProperties(info.format);
	if (info.tiling == VK_IMAGE_TILING_LINEAR && (formatProp.linearTilingFeatures & info.usage))
		return false;
	if (info.tiling == VK_IMAGE_TILING_OPTIMAL && (formatProp.optimalTilingFeatures & info.usage != info.usage))
		return false;

	// Image created must have flag VK_IMAGE_LAYOUT_UNDEFINED
	VkImageLayout layout = info.initialLayout;
	m_info = info;
	m_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	CHECK_VK_ERROR(vkCreateImage(GetDevice()->GetDeviceHandle(), &m_info, nullptr, &m_image));
	m_pMemKey = DeviceMemMgr()->AllocateImageMemChunk(GetSelfSharedPtr(), memoryPropertyFlag);

	m_info.initialLayout = layout;
	m_memProperty = memoryPropertyFlag;

	CreateImageView();
	CreateSampler();
	EnsureImageLayout();

	m_descriptorImgInfo = { m_sampler, m_view, m_info.initialLayout };

	return true;
}

bool Image::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, const gli::texture& gliTex, const VkImageCreateInfo& info, uint32_t memoryPropertyFlag)
{
	if (!Image::Init(pDevice, pSelf, info, memoryPropertyFlag))
		return false;

	UpdateByteStream(gliTex);
	return true;
}

bool Image::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, VkImage img)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_image = img;

	CreateImageView();
	return true;
}

VkMemoryRequirements Image::GetMemoryReqirments() const
{
	VkMemoryRequirements reqs;
	vkGetImageMemoryRequirements(GetDevice()->GetDeviceHandle(), GetDeviceHandle(), &reqs);
	return reqs;
}

void Image::BindMemory(VkDeviceMemory memory, uint32_t offset) const
{
	CHECK_VK_ERROR(vkBindImageMemory(GetDevice()->GetDeviceHandle(), GetDeviceHandle(), memory, offset));
}

void Image::EnsureImageLayout()
{
	if (m_info.initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		return;

	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();
	pCmdBuffer->StartRecording();

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = m_info.mipLevels;
	subresourceRange.layerCount = m_info.arrayLayers;

	if (m_info.usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

	VkImageMemoryBarrier imgBarrier = {};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imgBarrier.srcAccessMask = 0;
	imgBarrier.newLayout = m_info.initialLayout;
	imgBarrier.dstAccessMask |= VulkanUtil::GetAccessFlagByLayout(m_info.initialLayout);

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		{}, {}, { imgBarrier }
	);

	pCmdBuffer->EndRecording();

	GlobalGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
}

std::shared_ptr<StagingBuffer> Image::PrepareStagingBuffer(const gli::texture& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	std::shared_ptr<StagingBuffer> pStagingBuffer = StagingBuffer::Create(m_pDevice, gliTex.size());
	pStagingBuffer->UpdateByteStream(gliTex.data(), 0, gliTex.size(), (VkPipelineStageFlagBits)0, 0);

	// Barrier for host data copy & transfer src
	std::vector<VkBufferMemoryBarrier> bufferBarriers(1);
	bufferBarriers[0] = {};
	bufferBarriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	bufferBarriers[0].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	bufferBarriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	bufferBarriers[0].buffer = pStagingBuffer->GetDeviceHandle();
	bufferBarriers[0].offset = 0;
	bufferBarriers[0].size = gliTex.size();

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		{},
		bufferBarriers,
		{}
	);

	return pStagingBuffer;
}

void Image::ChangeImageLayoutBeforeCopy(const gli::texture& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	//Barrier for layout change from undefined to transfer dst
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = gliTex.levels();
	subresourceRange.layerCount = m_info.arrayLayers;

	std::vector<VkImageMemoryBarrier> imgBarriers(1);
	imgBarriers[0] = {};
	imgBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarriers[0].image = GetDeviceHandle();
	imgBarriers[0].subresourceRange = subresourceRange;
	imgBarriers[0].oldLayout = m_info.initialLayout;
	imgBarriers[0].srcAccessMask |= VulkanUtil::GetAccessFlagByLayout(m_info.initialLayout);
	imgBarriers[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imgBarriers[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		{},
		{},
		imgBarriers
	);
}

void Image::ExecuteCopy(const gli::texture& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	gli::texture2d gliTex2D = (gli::texture2d)gliTex;

	// Prepare copy info
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (uint32_t i = 0; i < gliTex.levels(); i++)
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = i;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = gliTex2D[i].extent().x;
		bufferCopyRegion.imageExtent.height = gliTex2D[i].extent().y;
		bufferCopyRegion.imageExtent.depth = gliTex2D[i].extent().z;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);

		offset += static_cast<uint32_t>(gliTex2D[i].size());
	}

	// Do copy
	vkCmdCopyBufferToImage(pCmdBuffer->GetDeviceHandle(),
		pStagingBuffer->GetDeviceHandle(),
		GetDeviceHandle(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		bufferCopyRegions.size(),
		bufferCopyRegions.data());
}

void Image::ChangeImageLayoutAfterCopy(const gli::texture& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	//Barrier for layout change from undefined to transfer dst
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = gliTex.levels();
	subresourceRange.layerCount = m_info.arrayLayers;

	std::vector<VkImageMemoryBarrier> imgBarriers(1);
	imgBarriers[0] = {};
	imgBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarriers[0].image = GetDeviceHandle();
	imgBarriers[0].subresourceRange = subresourceRange;
	imgBarriers[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imgBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imgBarriers[0].newLayout = m_info.initialLayout;
	imgBarriers[0].dstAccessMask |= VulkanUtil::GetAccessFlagByLayout(m_info.initialLayout);

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		{},
		{},
		imgBarriers
	);
}

void Image::UpdateByteStream(const gli::texture& gliTex)
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();
	pCmdBuffer->StartRecording();

	std::shared_ptr<StagingBuffer> pStagingBuffer = PrepareStagingBuffer(gliTex, pCmdBuffer);

	ChangeImageLayoutBeforeCopy(gliTex, pCmdBuffer);

	ExecuteCopy(gliTex, pStagingBuffer, pCmdBuffer);

	ChangeImageLayoutAfterCopy(gliTex, pCmdBuffer);

	pCmdBuffer->EndRecording();

	GlobalGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
}

void Image::CreateSampler()
{
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = m_info.mipLevels;
	//if (GetPhysicalDevice()->GetPhysicalDeviceFeatures().samplerAnisotropy)
	//{
	//	sampler.maxAnisotropy = GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
	//	sampler.anisotropyEnable = VK_TRUE;
	//}
	//else
	{
		samplerCreateInfo.maxAnisotropy = 1.0;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;
	}
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	CHECK_VK_ERROR(vkCreateSampler(GetDevice()->GetDeviceHandle(), &samplerCreateInfo, nullptr, &m_sampler));
}

void Image::CreateImageView()
{
	m_viewInfo = {};
	m_viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	m_viewInfo.image = m_image;
	m_viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	m_viewInfo.format = m_info.format;
	m_viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	m_viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	m_viewInfo.subresourceRange.baseArrayLayer = 0;
	m_viewInfo.subresourceRange.layerCount = m_info.arrayLayers;
	m_viewInfo.subresourceRange.baseMipLevel = 0;
	m_viewInfo.subresourceRange.levelCount = m_info.mipLevels;

	CHECK_VK_ERROR(vkCreateImageView(m_pDevice->GetDeviceHandle(), &m_viewInfo, nullptr, &m_view));
}