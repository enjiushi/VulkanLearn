#pragma once

#include "Image.h"

class TextureCube : public Image
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<TextureCube>& pSelf, const gli::texture_cube& gliTexCube, VkFormat format);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<TextureCube>& pSelf, uint32_t width, uint32_t height, VkFormat format);
	void ExecuteCopy(const gli::texture& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;
	void CreateImageView() override;

public:
	static std::shared_ptr<TextureCube> Create(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format);
	static std::shared_ptr<TextureCube> CreateEmptyTextureCube(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, VkFormat format);
};