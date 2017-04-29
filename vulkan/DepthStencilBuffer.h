#pragma once

#include "Image.h"

class DepthStencilBuffer : public Image
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, VkFormat format, uint32_t width, uint32_t height);

public:
	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }

public:
	static std::shared_ptr<DepthStencilBuffer> Create(const std::shared_ptr<Device>& pDevice, VkFormat format, uint32_t width, uint32_t height);
	static std::shared_ptr<DepthStencilBuffer> Create(const std::shared_ptr<Device>& pDevice);

protected:
	uint32_t m_width;
	uint32_t m_height;
};