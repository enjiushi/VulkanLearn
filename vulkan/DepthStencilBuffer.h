#pragma once

#include "Image.h"

class DepthStencilBuffer : public Image
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, VkFormat format, uint32_t width, uint32_t height);

public:
	void EnsureImageLayout() override;

public:
	static std::shared_ptr<DepthStencilBuffer> Create(const std::shared_ptr<Device>& pDevice, VkFormat format, uint32_t width, uint32_t height);
	static std::shared_ptr<DepthStencilBuffer> Create(const std::shared_ptr<Device>& pDevice);
};