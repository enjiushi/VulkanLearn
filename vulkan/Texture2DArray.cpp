#include "Texture2DArray.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include "Queue.h"
#include "CommandBuffer.h"
#include "StagingBuffer.h"
#include "ImageView.h"

bool Texture2DArray::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2DArray>& pSelf, const GliImageWrapper& gliTextureArray, VkFormat format, VkImageLayout defaultLayout)
{
	m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	m_accessFlags = VK_ACCESS_SHADER_READ_BIT;

	uint32_t width = gliTextureArray.textures[0].extent().x;
	uint32_t height = gliTextureArray.textures[0].extent().y;
	uint32_t mipLevels = (uint32_t)gliTextureArray.textures[0].levels();
	uint32_t layers = (uint32_t)gliTextureArray.textures.size();

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	textureCreateInfo.arrayLayers = layers;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = defaultLayout;
	textureCreateInfo.mipLevels = mipLevels;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (!Image::Init(pDevice, pSelf, gliTextureArray, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	return true;
}

bool Texture2DArray::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2DArray>& pSelf, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format, VkImageLayout defaultLayout)
{
	m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	m_accessFlags = VK_ACCESS_SHADER_READ_BIT;

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
	textureCreateInfo.arrayLayers = layers;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = defaultLayout;
	textureCreateInfo.mipLevels = mipLevels;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (!Image::Init(pDevice, pSelf, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	return true;
}

bool Texture2DArray::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2DArray>& pSelf, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format, VkImageUsageFlags usage, VkImageLayout defaultLayout)
{
	m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	m_accessFlags = VK_ACCESS_SHADER_READ_BIT;

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = usage;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
	textureCreateInfo.arrayLayers = layers;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = defaultLayout;
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
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, { {gliTextureArray} }, format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Texture2DArray> Texture2DArray::Create(const std::shared_ptr<Device>& pDevice, const GliImageWrapper& gliTextureArray, VkFormat format)
{
	std::shared_ptr<Texture2DArray> pTexture = std::make_shared<Texture2DArray>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, gliTextureArray, format, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
		return pTexture;
	return nullptr;
}

void Texture2DArray::InsertTexture(const gli::texture2d& texture, uint32_t layer)
{
	UpdateByteStream({ {texture} }, layer);
}

std::shared_ptr<StagingBuffer> Texture2DArray::PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
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

	return pStagingBuffer;
}

void Texture2DArray::ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
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

void Texture2DArray::ExecuteCopy(const GliImageWrapper& gliTex, uint32_t layer, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
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

std::shared_ptr<ImageView> Texture2DArray::CreateDefaultImageView() const
{
	VkImageViewCreateInfo imgViewCreateInfo = {};
	imgViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewCreateInfo.image = m_image;
	imgViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	imgViewCreateInfo.format = m_info.format;
	imgViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imgViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imgViewCreateInfo.subresourceRange.layerCount = m_info.arrayLayers;
	imgViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imgViewCreateInfo.subresourceRange.levelCount = m_info.mipLevels;

	return ImageView::Create(GetDevice(), imgViewCreateInfo);
}