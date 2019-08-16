#include "Image.h"
#include "GlobalDeviceObjects.h"
#include "DeviceMemoryManager.h"
#include "SwapChain.h"
#include "StagingBuffer.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Queue.h"
#include "VulkanUtil.h"
#include "ImageView.h"
#include "Sampler.h"

Image::~Image()
{
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

	EnsureImageLayout();

	m_bytesPerPixel = VulkanUtil::GetBytesFromFormat(m_info.format);

	return true;
}

bool Image::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, const GliImageWrapper& gliTex, const VkImageCreateInfo& info, uint32_t memoryPropertyFlag)
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

	//m_bytesPerPixel = VulkanUtil::GetBytesFromFormat(m_info.format);

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

	// Don't change image layout if it's color or depth stencil attachement
	// Since their actual raw layers are gonna changed by render pass
	if (m_info.initialLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || m_info.initialLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		return;

	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();
	pCmdBuffer->StartPrimaryRecording();

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = m_info.mipLevels;
	subresourceRange.layerCount = m_info.arrayLayers;

	if (m_info.format == VK_FORMAT_D24_UNORM_S8_UINT
		|| m_info.format == VK_FORMAT_D32_SFLOAT_S8_UINT)
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

	if (m_info.format == VK_FORMAT_D32_SFLOAT)
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	VkImageMemoryBarrier imgBarrier = {};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imgBarrier.srcAccessMask = 0;
	imgBarrier.newLayout = m_info.initialLayout;
	imgBarrier.dstAccessMask = 0;

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		{}, {}, { imgBarrier }
	);

	pCmdBuffer->EndPrimaryRecording();

	GlobalGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
}

void Image::UpdateByteStream(const GliImageWrapper& gliTex)
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();
	pCmdBuffer->StartPrimaryRecording();

	std::shared_ptr<StagingBuffer> pStagingBuffer = PrepareStagingBuffer(gliTex, pCmdBuffer);

	ExecuteCopy(gliTex, pStagingBuffer, pCmdBuffer);

	pCmdBuffer->EndPrimaryRecording();

	GlobalGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
}

void Image::UpdateByteStream(const GliImageWrapper& gliTex, uint32_t layer)
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();
	pCmdBuffer->StartPrimaryRecording();

	std::shared_ptr<StagingBuffer> pStagingBuffer = PrepareStagingBuffer(gliTex, pCmdBuffer);

	ExecuteCopy(gliTex, layer, pStagingBuffer, pCmdBuffer);

	pCmdBuffer->EndPrimaryRecording();

	GlobalGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
}

std::shared_ptr<Sampler> Image::CreateLinearRepeatSampler() const
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

	return Sampler::Create(GetDevice(), samplerCreateInfo);
}

std::shared_ptr<Sampler> Image::CreateNearestRepeatSampler() const
{
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
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

	return Sampler::Create(GetDevice(), samplerCreateInfo);
}

std::shared_ptr<Sampler> Image::CreateLinearClampToEdgeSampler() const
{
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
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

	return Sampler::Create(GetDevice(), samplerCreateInfo);
}

std::shared_ptr<ImageView> Image::CreateDefaultImageView() const
{
	VkImageViewCreateInfo imgViewCreateInfo = {};
	imgViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewCreateInfo.image = m_image;
	imgViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	imgViewCreateInfo.format = m_info.format;
	imgViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imgViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imgViewCreateInfo.subresourceRange.layerCount = m_info.arrayLayers;
	imgViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imgViewCreateInfo.subresourceRange.levelCount = m_info.mipLevels;

	return ImageView::Create(GetDevice(), imgViewCreateInfo);
}