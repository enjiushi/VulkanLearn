#pragma once

#include "DeviceObjectBase.h"

class Image;
class DepthStencilBuffer;
class RenderPass;
class Texture2D;

class FrameBuffer : public DeviceObjectBase<FrameBuffer>
{
public:
	static const VkFormat OFFSCREEN_COLOR_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
	static const VkFormat OFFSCREEN_HDR_COLOR_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	static const VkFormat OFFSCREEN_DEPTH_STENCIL_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT;

public:
	~FrameBuffer();

protected:
	bool Init(
		const std::shared_ptr<Device>& pDevice, 
		const std::shared_ptr<FrameBuffer>& pSelf,
		const std::shared_ptr<Image>& pImage,
		const std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer,
		const std::shared_ptr<RenderPass>& pRenderPass);

public:
	VkFramebuffer GetDeviceHandle() { return m_framebuffer; }
	VkFramebufferCreateInfo GetFramebufferInfo() const { return m_info; }
	void ExtractContent(const std::shared_ptr<Image>& pImage, uint32_t baseMipLevel = 0, uint32_t numMipLevels = 1, uint32_t baseLayer = 0, uint32_t numLayers = 1);

public:
	static std::shared_ptr<FrameBuffer> Create(
		const std::shared_ptr<Device>& pDevice,
		const std::shared_ptr<Image>& pImage,
		const std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer,
		const std::shared_ptr<RenderPass>& pRenderPass);

	static std::shared_ptr<FrameBuffer> CreateOffScreenFrameBuffer(
		const std::shared_ptr<Device>& pDevice, 
		uint32_t width, uint32_t height, 
		const std::shared_ptr<RenderPass>& pRenderPass);

protected:
	VkFramebuffer						m_framebuffer;
	VkFramebufferCreateInfo				m_info;

	std::vector<VkImageView>			m_imageViews;
	std::shared_ptr<Image>				m_pImage;
	std::shared_ptr<DepthStencilBuffer>	m_pDepthStencilBuffer;
	std::shared_ptr<RenderPass>			m_pRenderPass;
};