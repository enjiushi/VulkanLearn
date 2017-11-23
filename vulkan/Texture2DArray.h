#pragma once

#include "Image.h"

class Texture2DArray : public Image
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2DArray>& pSelf, const gli::texture2d_array& gliTextureArray, VkFormat format);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2DArray>& pSelf, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format);
	void ExecuteCopy(const gli::texture& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;
	void CreateImageView() override;

public:
	static std::shared_ptr<Texture2DArray> Create(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format);
	static std::shared_ptr<Texture2DArray> CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, uint32_t layers, VkFormat format);
	static std::shared_ptr<Texture2DArray> CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t layers, VkFormat format);
};