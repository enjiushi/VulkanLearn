#include "Framebuffer.h"
#include "Image.h"
#include "DepthStencilBuffer.h"
#include "RenderPass.h"
#include "Texture2D.h"

FrameBuffer::~FrameBuffer()
{
	vkDestroyFramebuffer(GetDevice()->GetDeviceHandle(), m_framebuffer, nullptr);
}

bool FrameBuffer::Init(
	const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<FrameBuffer>& pSelf,
	const std::shared_ptr<Image>& pImage,
	const std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer,
	const std::shared_ptr<RenderPass>& pRenderPass)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_pImage = pImage;
	m_pDepthStencilBuffer = pDepthStencilBuffer;
	m_pRenderPass = pRenderPass;

	ASSERTION(m_pImage != nullptr);
	m_imageViews.push_back(m_pImage->GetViewDeviceHandle());
	if (m_pDepthStencilBuffer != nullptr)
		m_imageViews.push_back(m_pDepthStencilBuffer->GetViewDeviceHandle());

	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	m_info.attachmentCount = m_imageViews.size();
	m_info.pAttachments = m_imageViews.data();
	m_info.layers = 1;
	m_info.width = m_pImage->GetImageInfo().extent.width;
	m_info.height = m_pImage->GetImageInfo().extent.height;
	m_info.renderPass = m_pRenderPass->GetDeviceHandle();

	RETURN_FALSE_VK_RESULT(vkCreateFramebuffer(GetDevice()->GetDeviceHandle(), &m_info, nullptr, &m_framebuffer));

	return true;
}

std::shared_ptr<FrameBuffer> FrameBuffer::Create(
	const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<Image>& pImage,
	const std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer,
	const std::shared_ptr<RenderPass>& pRenderPass)
{
	std::shared_ptr<FrameBuffer> pFramebuffer = std::make_shared<FrameBuffer>();
	if (pFramebuffer.get() && pFramebuffer->Init(pDevice, pFramebuffer, pImage, pDepthStencilBuffer, pRenderPass))
		return pFramebuffer;
	return nullptr;
}

std::shared_ptr<FrameBuffer> FrameBuffer::CreateOffScreenFrameBuffer(
	const std::shared_ptr<Device>& pDevice,
	uint32_t width, uint32_t height,
	const std::shared_ptr<RenderPass>& pRenderPass)
{
	std::shared_ptr<Texture2D> pOffScreenTex = Texture2D::CreateOffscreenTexture(pDevice, width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
	std::shared_ptr<DepthStencilBuffer> pDSBuffer = DepthStencilBuffer::Create(pDevice, VK_FORMAT_D32_SFLOAT_S8_UINT, width, height);

	std::shared_ptr<FrameBuffer> pFramebuffer = std::make_shared<FrameBuffer>();
	if (pFramebuffer.get() && pFramebuffer->Init(pDevice, pFramebuffer, pOffScreenTex, pDSBuffer, pRenderPass))
		return pFramebuffer;
	return nullptr;
}