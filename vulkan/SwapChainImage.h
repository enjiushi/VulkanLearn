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
};