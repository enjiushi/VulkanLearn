#include "Texture2D.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include "Queue.h"
#include "CommandBuffer.h"
#include "StagingBuffer.h"

bool Texture2D::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2D>& pSelf, const gli::texture& gliTex, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format)
{
	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	textureCreateInfo.arrayLayers = 1;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	textureCreateInfo.mipLevels = mipLevels;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (!Image::Init(pDevice, pSelf, gliTex, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	return true;
}

bool Texture2D::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2D>& pSelf, const gli::texture2d& gliTex2d, VkFormat format)
{
	uint32_t width = static_cast<uint32_t>(gliTex2d[0].extent().x);
	uint32_t height = static_cast<uint32_t>(gliTex2d[0].extent().y);
	uint32_t mipLevels = static_cast<uint32_t>(gliTex2d.levels());

	return Init(pDevice, pSelf, gliTex2d, width, height, mipLevels, format);
}

bool Texture2D::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2D>& pSelf, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage)
{
	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = usage;
	textureCreateInfo.arrayLayers = 1;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	textureCreateInfo.mipLevels = 1;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (!Image::Init(pDevice, pSelf, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	return true;
}

std::shared_ptr<Texture2D> Texture2D::Create(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format)
{
	gli::texture2d gliTex2d(gli::load(path.c_str()));
	std::shared_ptr<Texture2D> pTexture = std::make_shared<Texture2D>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, gliTex2d, format))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Texture2D> Texture2D::CreateEmptyTexture(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, VkFormat format)
{
	std::shared_ptr<Texture2D> pTexture = std::make_shared<Texture2D>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, width, height, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		return pTexture;
	return nullptr;
}

std::shared_ptr<Texture2D> Texture2D::CreateOffscreenTexture(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, VkFormat format)
{
	std::shared_ptr<Texture2D> pTexture = std::make_shared<Texture2D>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, width, height, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
		return pTexture;
	return nullptr;
}

void Texture2D::EnsureImageLayout()
{
}