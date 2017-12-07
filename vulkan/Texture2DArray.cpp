#include "Texture2DArray.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include "Queue.h"
#include "CommandBuffer.h"
#include "StagingBuffer.h"

bool Texture2DArray::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2DArray>& pSelf, const GliImageWrapper& gliTextureArray, VkFormat format)
{
	m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	m_accessFlags = VK_ACCESS_SHADER_READ_BIT;

	uint32_t width = gliTextureArray.textures[0].extent().x;
	uint32_t height = gliTextureArray.textures[0].extent().y;
	uint32_t mipLevels = gliTextureArray.textures[0].levels();
	uint32_t layers = gliTextureArray.textures[0].layers();

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	textureCreateInfo.arrayLayers = layers;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	textureCreateInfo.mipLevels = mipLevels;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (!Image::Init(pDevice, pSelf, gliTextureArray, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	return true;
}

bool Texture2DArray::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2DArray>& pSelf, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format)
{
	m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	m_accessFlags = VK_ACCESS_SHADER_READ_BIT;

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.flags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
	textureCreateInfo.arrayLayers = layers;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	textureCreateInfo.mipLevels = mipLevels;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (!Image::Init(pDevice, pSelf, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	return true;
}

std::shared_ptr<Texture2DArray> Texture2DArray::Create(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format)
{
	gli::texture2d_array gliTextureArray(gli::load(path.c_str()));
	std::shared_ptr<Texture2DArray> pTexture = std::make_shared<Texture2DArray>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, { {gliTextureArray} }, format))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Texture2DArray> Texture2DArray::CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, uint32_t layers, VkFormat format)
{
	std::shared_ptr<Texture2DArray> pTexture = std::make_shared<Texture2DArray>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, width, height, 1, layers, format))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Texture2DArray> Texture2DArray::CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format)
{
	std::shared_ptr<Texture2DArray> pTexture = std::make_shared<Texture2DArray>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, width, height, mipLevels, layers, format))
		return pTexture;
	return nullptr;
}

std::shared_ptr<StagingBuffer> Texture2DArray::PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	// Get total bytes of texture vector
	uint32_t total_bytes = 0;
	for (uint32_t i = 0; i < gliTex.textures.size(); i++)
		total_bytes += gliTex.textures[i].size();

	// Copy all bytes of texture vector into a buffer
	uint32_t offset = 0;
	uint8_t* buf = new uint8_t[total_bytes];
	for (uint32_t i = 0; i < gliTex.textures.size(); i++)
	{
		memcpy_s(buf, total_bytes - offset, gliTex.textures[i].data(), gliTex.textures[i].size());
		offset += gliTex.textures[i].size();
		buf += gliTex.textures[i].size();
	}

	std::shared_ptr<StagingBuffer> pStagingBuffer = StagingBuffer::Create(m_pDevice, total_bytes);
	pStagingBuffer->UpdateByteStream(buf, 0, total_bytes);

	return pStagingBuffer;
}


void Texture2DArray::ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	for (uint32_t i = 0; i < gliTex.textures.size(); i++)
	{
		// Prepare copy info
		std::vector<VkBufferImageCopy> bufferCopyRegions;
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
}

void Texture2DArray::CreateImageView()
{
	m_viewInfo = {};
	m_viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	m_viewInfo.image = m_image;
	m_viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	m_viewInfo.format = m_info.format;
	m_viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	m_viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	m_viewInfo.subresourceRange.baseArrayLayer = 0;
	m_viewInfo.subresourceRange.layerCount = m_info.arrayLayers;
	m_viewInfo.subresourceRange.baseMipLevel = 0;
	m_viewInfo.subresourceRange.levelCount = m_info.mipLevels;

	CHECK_VK_ERROR(vkCreateImageView(m_pDevice->GetDeviceHandle(), &m_viewInfo, nullptr, &m_view));
}