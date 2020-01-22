#pragma once

#include "Image.h"

class Texture2DArray : public Image
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2DArray>& pSelf, const GliImageWrapper& gliTextureArray, VkFormat format, VkImageLayout defaultLayout);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2DArray>& pSelf, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format, VkImageLayout defaultLayout);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2DArray>& pSelf, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format, VkImageUsageFlags usage, VkImageLayout defaultLayout);

public:
	static std::shared_ptr<Texture2DArray> Create(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format);
	static std::shared_ptr<Texture2DArray> Create(const std::shared_ptr<Device>& pDevice, const GliImageWrapper& gliTextureArray, VkFormat format);
	static std::shared_ptr<Texture2DArray> CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, uint32_t layers, VkFormat format);
	static std::shared_ptr<Texture2DArray> CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, uint32_t layers, VkFormat format, VkImageLayout defaultLayout);
	static std::shared_ptr<Texture2DArray> CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format);
	static std::shared_ptr<Texture2DArray> CreateMipmapOffscreenTexture(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, uint32_t layers, VkFormat format);

public:
	void InsertTexture(const gli::texture2d& texture, uint32_t layer) override;
	std::shared_ptr<ImageView> CreateDefaultImageView() const override;

protected:
	std::shared_ptr<StagingBuffer> PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;
	void ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;
	void ExecuteCopy(const GliImageWrapper& gliTex, uint32_t layer, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;
};