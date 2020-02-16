#pragma once

#include "Image.h"
#include <string>
#include <gli\gli.hpp>

class CommandBuffer;
class StagingBuffer;

class Texture2D : public Image
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2D>& pSelf, const GliImageWrapper& gliTex, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageLayout layout);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2D>& pSelf, const GliImageWrapper& gliTex2d, VkFormat format, VkImageLayout layout);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2D>& pSelf, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageLayout layout);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2D>& pSelf, uint32_t width, uint32_t height, uint32_t mips, VkFormat format, VkImageUsageFlags usage, VkImageLayout layout);

public:
	static std::shared_ptr<Texture2D> Create(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format);
	static std::shared_ptr<Texture2D> Create(const std::shared_ptr<Device>& pDevice, const GliImageWrapper& gliTex2d, VkFormat format);
	static std::shared_ptr<Texture2D> CreateMipmapOffscreenTexture(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, VkFormat format, VkImageLayout layout);

protected:
	std::shared_ptr<StagingBuffer> PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;
	void ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;
	void ExecuteCopy(const GliImageWrapper& gliTex, uint32_t layer, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;
};