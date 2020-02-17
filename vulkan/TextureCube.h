#pragma once

#include "Image.h"

class TextureCube : public Image
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<TextureCube>& pSelf, const GliImageWrapper& gliTexCube, VkFormat format);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<TextureCube>& pSelf, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format);

public:
	static std::shared_ptr<TextureCube> CreateCubeTexture(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format);

public:
	std::shared_ptr<ImageView> CreateDefaultImageView() const override;

protected:
	std::shared_ptr<StagingBuffer> PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;
	void ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;
	uint32_t ExecuteCopy(const GliImageWrapper& gliTex, uint32_t layer, const std::shared_ptr<StagingBuffer>& pStagingBuffer, uint32_t offset, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;
};