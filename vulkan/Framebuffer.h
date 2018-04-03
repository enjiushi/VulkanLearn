#pragma once

#include "DeviceObjectBase.h"

class Image;
class DepthStencilBuffer;
class RenderPass;
class Texture2D;
class ImageView;

class FrameBuffer : public DeviceObjectBase<FrameBuffer>
{
public:
	~FrameBuffer();

protected:
	bool Init(
		const std::shared_ptr<Device>& pDevice, 
		const std::shared_ptr<FrameBuffer>& pSelf,
		const std::vector<std::shared_ptr<Image>>& pImage,
		const std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer,
		const std::shared_ptr<RenderPass>& pRenderPass);

public:
	VkFramebuffer GetDeviceHandle() { return m_framebuffer; }
	VkFramebufferCreateInfo GetFramebufferInfo() const { return m_info; }
	std::shared_ptr<RenderPass> GetRenderPass() const { return m_pRenderPass; }
	void ExtractContent(const std::shared_ptr<Image>& pImage, uint32_t index = 0);
	void ExtractContent(const std::shared_ptr<Image>& pImage, uint32_t baseMipLevel, uint32_t numMipLevels, uint32_t baseLayer, uint32_t numLayers, uint32_t index = 0);
	void ExtractContent(const std::shared_ptr<Image>& pImage, uint32_t baseMipLevel, uint32_t numMipLevels, uint32_t baseLayer, uint32_t numLayers, uint32_t width, uint32_t height, uint32_t index = 0);
	std::shared_ptr<Image> GetColorTarget(uint32_t index) const { return m_images[index]; }
	std::shared_ptr<DepthStencilBuffer> GetDepthStencilTarget() const { return m_pDepthStencilBuffer; }

public:
	static std::shared_ptr<FrameBuffer> Create(
		const std::shared_ptr<Device>& pDevice,
		const std::shared_ptr<Image>& pImage,
		const std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer,
		const std::shared_ptr<RenderPass>& pRenderPass);

	static std::shared_ptr<FrameBuffer> Create(
		const std::shared_ptr<Device>& pDevice,
		const std::vector<std::shared_ptr<Image>>& images,
		const std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer,
		const std::shared_ptr<RenderPass>& pRenderPass);

	static std::shared_ptr<FrameBuffer> CreateOffScreenFrameBuffer(
		const std::shared_ptr<Device>& pDevice, 
		uint32_t width, uint32_t height, 
		const std::shared_ptr<RenderPass>& pRenderPass);

protected:
	VkFramebuffer								m_framebuffer;
	VkFramebufferCreateInfo						m_info;

	std::vector<std::shared_ptr<ImageView>>		m_imageViews;
	std::vector<std::shared_ptr<Image>>			m_images;
	std::shared_ptr<DepthStencilBuffer>			m_pDepthStencilBuffer;
	std::shared_ptr<RenderPass>					m_pRenderPass;
};