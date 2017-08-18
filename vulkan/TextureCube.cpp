#include "TextureCube.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include "Queue.h"
#include "CommandBuffer.h"
#include "StagingBuffer.h"

bool TextureCube::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<TextureCube>& pSelf, const gli::texture_cube& gliTexCube, VkFormat format)
{
	uint32_t width = gliTexCube.extent().x;
	uint32_t height = gliTexCube.extent().y;
	uint32_t mipLevels = gliTexCube.levels();

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	textureCreateInfo.arrayLayers = gliTexCube.faces();
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

bool TextureCube::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<TextureCube>& pSelf, uint32_t width, uint32_t height, VkFormat format)
{
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
	textureCreateInfo.mipLevels = 1;
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
	std::shared_ptr<TextureCube> pTexture = std::make_shared<TextureCube>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, gliTexCube, format))
		return pTexture;
	return nullptr;
}

std::shared_ptr<TextureCube> TextureCube::CreateEmptyTextureCube(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, VkFormat format)
{
	std::shared_ptr<TextureCube> pTexture = std::make_shared<TextureCube>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, width, height, format))
		return pTexture;
	return nullptr;
}

void TextureCube::ExecuteCopy(const gli::texture& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	gli::texture_cube gliTexCube = (gli::texture_cube)gliTex;

	// Prepare copy info
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (uint32_t face = 0; face < 6; face++)
	{
		for (uint32_t i = 0; i < gliTex.levels(); i++)
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

	// Do copy
	vkCmdCopyBufferToImage(pCmdBuffer->GetDeviceHandle(),
		pStagingBuffer->GetDeviceHandle(),
		GetDeviceHandle(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		bufferCopyRegions.size(),
		bufferCopyRegions.data());
}

void TextureCube::CreateImageView()
{
	m_viewInfo = {};
	m_viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	m_viewInfo.image = m_image;
	m_viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	m_viewInfo.format = m_info.format;
	m_viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	m_viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	m_viewInfo.subresourceRange.baseArrayLayer = 0;
	m_viewInfo.subresourceRange.layerCount = m_info.arrayLayers;
	m_viewInfo.subresourceRange.baseMipLevel = 0;
	m_viewInfo.subresourceRange.levelCount = m_info.mipLevels;

	CHECK_VK_ERROR(vkCreateImageView(m_pDevice->GetDeviceHandle(), &m_viewInfo, nullptr, &m_view));
}