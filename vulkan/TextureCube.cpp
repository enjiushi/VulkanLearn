#include "TextureCube.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include "Queue.h"
#include "CommandBuffer.h"
#include "StagingBuffer.h"
#include "ImageView.h"

bool TextureCube::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<TextureCube>& pSelf, const GliImageWrapper& gliTexCube, VkFormat format)
{
	m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	m_accessFlags = VK_ACCESS_SHADER_READ_BIT;

	uint32_t width = gliTexCube.textures[0].extent().x;
	uint32_t height = gliTexCube.textures[0].extent().y;
	uint32_t mipLevels = gliTexCube.textures[0].levels();

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	textureCreateInfo.arrayLayers = gliTexCube.textures[0].faces();
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	textureCreateInfo.mipLevels = mipLevels;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (!Image::Init(pDevice, pSelf, gliTexCube, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	return true;
}

bool TextureCube::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<TextureCube>& pSelf, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format)
{
	m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	m_accessFlags = VK_ACCESS_SHADER_READ_BIT;

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	textureCreateInfo.arrayLayers = 6;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
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

std::shared_ptr<TextureCube> TextureCube::Create(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format)
{
	gli::texture_cube gliTexCube(gli::load(path.c_str()));
	GliImageWrapper wrapper = { {gliTexCube} };
	std::shared_ptr<TextureCube> pTexture = std::make_shared<TextureCube>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, wrapper, format))
		return pTexture;
	return nullptr;
}

std::shared_ptr<TextureCube> TextureCube::CreateEmptyTextureCube(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, VkFormat format)
{
	std::shared_ptr<TextureCube> pTexture = std::make_shared<TextureCube>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, width, height, 1, format))
		return pTexture;
	return nullptr;
}

std::shared_ptr<TextureCube> TextureCube::CreateEmptyTextureCube(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format)
{
	std::shared_ptr<TextureCube> pTexture = std::make_shared<TextureCube>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, width, height, mipLevels, format))
		return pTexture;
	return nullptr;
}

std::shared_ptr<StagingBuffer> TextureCube::PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	std::shared_ptr<StagingBuffer> pStagingBuffer = StagingBuffer::Create(m_pDevice, gliTex.textures[0].size());
	pStagingBuffer->UpdateByteStream(gliTex.textures[0].data(), 0, gliTex.textures[0].size());

	return pStagingBuffer;
}

void TextureCube::ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	gli::texture_cube gliTexCube = (gli::texture_cube)gliTex.textures[0];

	// Prepare copy info
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (uint32_t face = 0; face < 6; face++)
	{
		for (uint32_t i = 0; i < gliTexCube.levels(); i++)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = i;
			bufferCopyRegion.imageSubresource.baseArrayLayer = face;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = gliTexCube[face][i].extent().x;
			bufferCopyRegion.imageExtent.height = gliTexCube[face][i].extent().y;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

			offset += static_cast<uint32_t>(gliTexCube[face][i].size());
		}
	}

	pCmdBuffer->CopyBufferImage(pStagingBuffer, GetSelfSharedPtr(), bufferCopyRegions);
}

void TextureCube::ExecuteCopy(const GliImageWrapper& gliTex, uint32_t layer, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	ASSERTION(gliTex.textures.size() == 1);

	gli::texture2d gliTex2d = (gli::texture2d)gliTex.textures[0];

	// Prepare copy info
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (uint32_t i = 0; i < gliTex2d.levels(); i++)
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = i;
		bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = gliTex2d[i].extent().x;
		bufferCopyRegion.imageExtent.height = gliTex2d[i].extent().y;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);

		offset += static_cast<uint32_t>(gliTex2d[i].size());
	}

	pCmdBuffer->CopyBufferImage(pStagingBuffer, GetSelfSharedPtr(), bufferCopyRegions);
}

std::shared_ptr<ImageView> TextureCube::CreateDefaultImageView() const
{
	VkImageViewCreateInfo imgViewCreateInfo = {};
	imgViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewCreateInfo.image = m_image;
	imgViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	imgViewCreateInfo.format = m_info.format;
	imgViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imgViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imgViewCreateInfo.subresourceRange.layerCount = m_info.arrayLayers;
	imgViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imgViewCreateInfo.subresourceRange.levelCount = m_info.mipLevels;

	return ImageView::Create(GetDevice(), imgViewCreateInfo);
}