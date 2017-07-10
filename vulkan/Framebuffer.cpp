#include "Framebuffer.h"
#include "Image.h"
#include "DepthStencilBuffer.h"
#include "RenderPass.h"

FrameBuffer::~FrameBuffer()
{
	vkDestroyFramebuffer(GetDevice()->GetDeviceHandle(), m_framebuffer, nullptr);
}

bool FrameBuffer::Init(
	const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<Image>& pImage,
	const std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer,
	const std::shared_ptr<RenderPass>& pRenderPass)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	m_pImage = pImage;
	m_pDepthStencilBuffer = pDepthStencilBuffer;
	m_pRenderPass = pRenderPass;

	m_imageViews =
	{
		m_pImage->GetViewDeviceHandle(),
		m_pDepthStencilBuffer->GetViewDeviceHandle()
	};

	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	m_info.attachmentCount = m_imageViews.size();
	m_info.pAttachments = m_imageViews.data();
	m_info.layers = 1;
	m_info.width = GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
	m_info.height = GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
	m_info.renderPass = m_pRenderPass->GetDeviceHandle();

	CHECK_VK_ERROR(vkCreateFramebuffer(GetDevice()->GetDeviceHandle(), &m_info, nullptr, &m_framebuffer));

	return true;
}

std::shared_ptr<FrameBuffer> FrameBuffer::Create(
	const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<Image>& pImage,
	const std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer,
	const std::shared_ptr<RenderPass>& pRenderPass)
{
	std::shared_ptr<FrameBuffer> pFramebuffer = std::make_shared<FrameBuffer>();
	if (pFramebuffer.get() && pFramebuffer->Init(pDevice, pImage, pDepthStencilBuffer, pRenderPass))
		return pFramebuffer;
	return nullptr;
}