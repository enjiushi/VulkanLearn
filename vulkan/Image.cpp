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

	// Skip this for now as it requires to enumerating all usage and pair it with format feature
	// Don't do it yet
	// The code below is totally incorrect
	
	/*VkFormatProperties formatProp = pDevice->GetPhysicalDevice()->GetPhysicalDeviceFormatProperties(info.format);
	if (info.tiling == VK_IMAGE_TILING_LINEAR && (formatProp.linearTilingFeatures & info.usage))
		return false;

	if ((info.tiling == VK_IMAGE_TILING_OPTIMAL) && ((formatProp.optimalTilingFeatures & info.usage) != info.usage))
		return false;*/

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

	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadGraphicPool()->AllocatePrimaryCommandBuffer();
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
	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadGraphicPool()->AllocatePrimaryCommandBuffer();
	pCmdBuffer->StartPrimaryRecording();

	std::shared_ptr<StagingBuffer> pStagingBuffer = PrepareStagingBuffer(gliTex, pCmdBuffer);

	ExecuteCopy(gliTex, pStagingBuffer, pCmdBuffer);

	pCmdBuffer->EndPrimaryRecording();

	GlobalGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
}

void Image::UpdateByteStream(const GliImageWrapper& gliTex, uint32_t layer)
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadGraphicPool()->AllocatePrimaryCommandBuffer();
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
	samplerCreateInfo.maxLod = (float)m_info.mipLevels;
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
	samplerCreateInfo.maxLod = (float)m_info.mipLevels;
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

std::shared_ptr<Sampler> Image::CreateLinearClampToBorderSampler(VkBorderColor borderColor) const
{
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = (float)m_info.mipLevels;
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
	samplerCreateInfo.borderColor = borderColor;

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
	samplerCreateInfo.maxLod = (float)m_info.mipLevels;
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

	return Sampler::Create(GetDevice(), samplerCreateInfo);
}

std::shared_ptr<ImageView> Image::CreateDefaultImageView() const
{
	VkImageViewCreateInfo imgViewCreateInfo = {};
	imgViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewCreateInfo.image = m_image;
	imgViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	imgViewCreateInfo.format = m_info.format;
	imgViewCreateInfo.viewType = m_info.arrayLayers == 1? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imgViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imgViewCreateInfo.subresourceRange.layerCount = m_info.arrayLayers;
	imgViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imgViewCreateInfo.subresourceRange.levelCount = m_info.mipLevels;

	return ImageView::Create(GetDevice(), imgViewCreateInfo);
}

void Image::InsertTexture(const gli::texture2d& texture, uint32_t layer)
{
	UpdateByteStream({ { texture } }, layer);
}

std::shared_ptr<StagingBuffer> Image::PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	// Get total bytes of texture vector
	uint32_t total_bytes = 0;
	for (uint32_t i = 0; i < gliTex.textures.size(); i++)
		total_bytes += (uint32_t)gliTex.textures[i].size();

	// Copy all bytes of texture vector into a buffer
	uint32_t offset = 0;
	uint8_t* buf = new uint8_t[total_bytes];
	for (uint32_t i = 0; i < gliTex.textures.size(); i++)
	{
		memcpy_s(buf + offset, total_bytes - offset, gliTex.textures[i].data(), gliTex.textures[i].size());
		offset += (uint32_t)gliTex.textures[i].size();
	}

	std::shared_ptr<StagingBuffer> pStagingBuffer = StagingBuffer::Create(m_pDevice, total_bytes);
	pStagingBuffer->UpdateByteStream(buf, 0, total_bytes);

	delete[] buf;

	return pStagingBuffer;
}

void Image::ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	std::vector<VkBufferImageCopy> bufferCopyRegions;

	for (uint32_t i = 0; i < gliTex.textures.size(); i++)
	{
		uint32_t offset = 0;

		for (uint32_t level = 0; level < gliTex.textures[i].levels(); level++)
		{
			gli::texture2d tex2d = (gli::texture2d)gliTex.textures[i];
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = level;
			bufferCopyRegion.imageSubresource.baseArrayLayer = i;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = tex2d[level].extent().x;
			bufferCopyRegion.imageExtent.height = tex2d[level].extent().y;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

			offset += static_cast<uint32_t>(tex2d[level].size());
		}
		pCmdBuffer->CopyBufferImage(pStagingBuffer, GetSelfSharedPtr(), bufferCopyRegions);
	}

	pCmdBuffer->CopyBufferImage(pStagingBuffer, GetSelfSharedPtr(), bufferCopyRegions);
}

void Image::ExecuteCopy(const GliImageWrapper& gliTex, uint32_t layer, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	ASSERTION(gliTex.textures.size() == 1);

	std::vector<VkBufferImageCopy> bufferCopyRegions;

	uint32_t offset = 0;

	for (uint32_t level = 0; level < gliTex.textures[0].levels(); level++)
	{
		gli::texture2d tex2d = (gli::texture2d)gliTex.textures[0];
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = level;
		bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = tex2d[level].extent().x;
		bufferCopyRegion.imageExtent.height = tex2d[level].extent().y;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);

		uint32_t size = (uint32_t)tex2d[level].size();
		offset += static_cast<uint32_t>(tex2d[level].size());
	}

	pCmdBuffer->CopyBufferImage(pStagingBuffer, GetSelfSharedPtr(), bufferCopyRegions);
}

std::shared_ptr<Image> Image::Create(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format)
{
	gli::texture2d gliTex2d(gli::load(path.c_str()));
	return Create(pDevice, { {gliTex2d} }, format);
}

std::shared_ptr<Image> Image::Create(const std::shared_ptr<Device>& pDevice, const GliImageWrapper& gliTex2d, VkFormat format)
{
	uint32_t width = static_cast<uint32_t>(gliTex2d.textures[0].extent().x);
	uint32_t height = static_cast<uint32_t>(gliTex2d.textures[0].extent().y);
	uint32_t mipLevels = static_cast<uint32_t>(gliTex2d.textures[0].levels());

	std::shared_ptr<Image> pTexture = std::make_shared<Image>();

	if (pTexture.get())
	{
		pTexture->m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		pTexture->m_accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
	}

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	textureCreateInfo.arrayLayers = 1;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	textureCreateInfo.mipLevels = 1;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (pTexture.get() && pTexture->Init(pDevice, pTexture, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Image> Image::CreateEmptyTexture(const std::shared_ptr<Device>& pDevice, const Vector3ui& size, VkFormat format)
{
	std::shared_ptr<Image> pTexture = std::make_shared<Image>();

	if (pTexture.get())
	{
		pTexture->m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		pTexture->m_accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
	}

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	textureCreateInfo.arrayLayers = 1;
	textureCreateInfo.extent.depth = size.z;
	textureCreateInfo.extent.width = size.x;
	textureCreateInfo.extent.height = size.y;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	textureCreateInfo.mipLevels = 1;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (pTexture.get() && pTexture->Init(pDevice, pTexture, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Image> Image::CreateEmptyTextureForCompute(const std::shared_ptr<Device>& pDevice, const Vector3ui& size, VkFormat format)
{
	std::shared_ptr<Image> pTexture = std::make_shared<Image>();

	if (pTexture.get())
	{
		pTexture->m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		pTexture->m_accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	}

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	textureCreateInfo.arrayLayers = 1;
	textureCreateInfo.extent.depth = size.z;
	textureCreateInfo.extent.width = size.x;
	textureCreateInfo.extent.height = size.y;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	textureCreateInfo.mipLevels = 1;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (pTexture.get() && pTexture->Init(pDevice, pTexture, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Image> Image::CreateOffscreenTexture(const std::shared_ptr<Device>& pDevice, const Vector3ui& size, VkFormat format)
{
	std::shared_ptr<Image> pTexture = std::make_shared<Image>();

	if (pTexture.get())
	{
		pTexture->m_accessStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		pTexture->m_accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
	}

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	textureCreateInfo.arrayLayers = 1;
	textureCreateInfo.extent.depth = size.z;
	textureCreateInfo.extent.width = size.x;
	textureCreateInfo.extent.height = size.y;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	textureCreateInfo.mipLevels = 1;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (pTexture.get() && pTexture->Init(pDevice, pTexture, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Image> Image::CreateOffscreenTexture(const std::shared_ptr<Device>& pDevice, const Vector3ui& size, VkFormat format, VkImageLayout layout)
{
	std::shared_ptr<Image> pTexture = std::make_shared<Image>();

	if (pTexture.get())
	{
		pTexture->m_accessStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		pTexture->m_accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
	}

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	textureCreateInfo.arrayLayers = 1;
	textureCreateInfo.extent.depth = size.z;
	textureCreateInfo.extent.width = size.x;
	textureCreateInfo.extent.height = size.y;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = layout;
	textureCreateInfo.mipLevels = 1;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (pTexture.get() && pTexture->Init(pDevice, pTexture, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Image> Image::CreateMipmapOffscreenTexture(const std::shared_ptr<Device>& pDevice, const Vector3ui& size, VkFormat format, VkImageLayout layout)
{
	std::shared_ptr<Image> pTexture = std::make_shared<Image>();

	if (pTexture.get())
	{
		pTexture->m_accessStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		pTexture->m_accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
	}

	uint32_t smaller = size.y < size.x ? size.y : size.x;

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	textureCreateInfo.arrayLayers = 1;
	textureCreateInfo.extent.depth = size.z;
	textureCreateInfo.extent.width = size.x;
	textureCreateInfo.extent.height = size.y;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = layout;
	textureCreateInfo.mipLevels = 1;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (pTexture.get() && pTexture->Init(pDevice, pTexture, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Image> Image::CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, const Vector3ui& size, uint32_t layers, VkFormat format)
{
	return CreateEmptyTexture2DArray(pDevice, size, 1, layers, format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

std::shared_ptr<Image> Image::CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, const Vector3ui& size, uint32_t layers, VkFormat format, VkImageLayout defaultLayout)
{
	return CreateEmptyTexture2DArray(pDevice, size, 1, layers, format, defaultLayout);
}

std::shared_ptr<Image> Image::CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, const Vector3ui& size, uint32_t mipLevels, uint32_t layers, VkFormat format)
{
	return CreateEmptyTexture2DArray(pDevice, size, mipLevels, layers, format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

std::shared_ptr<Image> Image::CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, const Vector3ui& size, uint32_t mipLevels, uint32_t layers, VkFormat format, VkImageLayout defaultLayout)
{
	std::shared_ptr<Image> pTexture = std::make_shared<Image>();

	if (pTexture.get())
	{
		pTexture->m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		pTexture->m_accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	textureCreateInfo.extent.depth = size.z;
	textureCreateInfo.extent.width = size.x;
	textureCreateInfo.extent.height = size.y;
	textureCreateInfo.arrayLayers = layers;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = defaultLayout;
	textureCreateInfo.mipLevels = mipLevels;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (pTexture.get() && pTexture->Init(pDevice, pTexture, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Image> Image::CreateMipmapOffscreenTexture(const std::shared_ptr<Device>& pDevice, const Vector3ui& size, uint32_t layers, VkFormat format)
{
	std::shared_ptr<Image> pTexture = std::make_shared<Image>();

	if (pTexture.get())
	{
		pTexture->m_accessStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		pTexture->m_accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
	}

	uint32_t smaller = size.y < size.x ? size.y : size.x;

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	textureCreateInfo.extent.depth = size.z;
	textureCreateInfo.extent.width = size.x;
	textureCreateInfo.extent.height = size.y;
	textureCreateInfo.arrayLayers = layers;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	textureCreateInfo.mipLevels = (uint32_t)std::log2(smaller) + 1;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (pTexture.get() && pTexture->Init(pDevice, pTexture, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return pTexture;
	return nullptr;
}