#pragma once

#include "DeviceObjectBase.h"

class Image;
class DepthStencilBuffer;
class RenderPass;

class Framebuffer : public DeviceObjectBase
{
public:
	~Framebuffer();

	bool Init(
		const std::shared_ptr<Device>& pDevice,
		const std::shared_ptr<Image>& pImage,
		const std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer,
		const std::shared_ptr<RenderPass>& pRenderPass);

public:
	VkFramebuffer GetDeviceHandle() { return m_framebuffer; }
	VkFramebufferCreateInfo GetFramebufferInfo() const { return m_info; }

public:
	static std::shared_ptr<Framebuffer> Create(
		const std::shared_ptr<Device>& pDevice,
		const std::shared_ptr<Image>& pImage,
		const std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer,
		const std::shared_ptr<RenderPass>& pRenderPass);

protected:
	VkFramebuffer						m_framebuffer;
	VkFramebufferCreateInfo				m_info;

	std::vector<VkImageView>			m_imageViews;
	std::shared_ptr<Image>				m_pImage;
	std::shared_ptr<DepthStencilBuffer>	m_pDepthStencilBuffer;
	std::shared_ptr<RenderPass>			m_pRenderPass;
};