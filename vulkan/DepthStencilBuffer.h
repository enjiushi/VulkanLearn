#pragma once

#include "Image.h"

class DepthStencilBuffer : public Image
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<DepthStencilBuffer>& pSelf, const VkImageCreateInfo& info);

public:
	std::shared_ptr<ImageView> CreateDefaultImageView() const override;
	std::shared_ptr<ImageView> CreateDepthSampleImageView() const;

protected:
	std::shared_ptr<StagingBuffer> PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override { return nullptr; };
	void ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override {};
	uint32_t ExecuteCopy(const GliImageWrapper& gliTex, uint32_t layer, const std::shared_ptr<StagingBuffer>& pStagingBuffer, uint32_t offset, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override { return 0; }
};