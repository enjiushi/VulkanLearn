#pragma once

#include "Image.h"

class SwapChainImage : public Image
{
public:
	static std::vector<std::shared_ptr<SwapChainImage>> Create(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SwapChain>& pSwapChain);

public:
	void EnsureImageLayout() override;

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SwapChainImage>& pSelf, VkImage rawImageHandle);
	static std::shared_ptr<SwapChainImage> Create(const std::shared_ptr<Device>& pDevice, VkImage rawImageHandle);

	std::shared_ptr<StagingBuffer> PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override { return nullptr; };
	uint32_t ExecuteCopy(const GliImageWrapper& gliTex, uint32_t layer, const std::shared_ptr<StagingBuffer>& pStagingBuffer, uint32_t offset, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override { return 0; }
	void ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override {};

};