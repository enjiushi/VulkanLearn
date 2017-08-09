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
	textureCreateInfo.arrayLayers = 6;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	textureCreateInfo.mipLevels = mipLevels;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (!Image::Init(pDevice, pSelf, gliTexCube, textureCreateInfo, format))
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

void TextureCube::EnsureImageLayout()
{
}