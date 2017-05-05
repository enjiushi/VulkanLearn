#pragma once

#include "Image.h"

class SwapChainImage : public Image
{
public:
	static std::vector<std::shared_ptr<SwapChainImage>> Create(const std::shared_ptr<Device>& pDevice);

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, VkImage rawImageHandle);
	static std::shared_ptr<SwapChainImage> Create(const std::shared_ptr<Device>& pDevice, VkImage rawImageHandle);
};