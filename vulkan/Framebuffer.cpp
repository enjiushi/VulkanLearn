#include "Framebuffer.h"
#include "Image.h"
#include "DepthStencilBuffer.h"
#include "RenderPass.h"
#include "Texture2D.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Queue.h"

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

void FrameBuffer::ExtractContent(const std::shared_ptr<Image>& pImage)
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();
	pCmdBuffer->StartRecording();

	VkImageMemoryBarrier fbBarrier = {};
	fbBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	fbBarrier.image = m_pImage->GetDeviceHandle();
	fbBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	fbBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	fbBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	fbBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	fbBarrier.subresourceRange =
	{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0, 1,
		0, 1
	};

	VkImageMemoryBarrier inputImgBarrier = {};
	inputImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	inputImgBarrier.image = pImage->GetDeviceHandle();
	inputImgBarrier.oldLayout = pImage->GetImageInfo().initialLayout;
	if (inputImgBarrier.oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		inputImgBarrier.srcAccessMask |= (VK_ACCESS_SHADER_READ_BIT);
	if (inputImgBarrier.oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		inputImgBarrier.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	inputImgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	inputImgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	inputImgBarrier.subresourceRange =
	{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0, 1,
		0, 1
	};

	std::vector<VkImageMemoryBarrier> barriers = { fbBarrier, inputImgBarrier };

	vkCmdPipelineBarrier
	(
		pCmdBuffer->GetDeviceHandle(),
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		barriers.size(), barriers.data()
	);

	VkImageCopy copy = {};

	copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copy.srcSubresource.baseArrayLayer = 0;
	copy.srcSubresource.layerCount = 1;
	copy.srcSubresource.mipLevel = 0;
	copy.srcOffset = { 0, 0, 0 };

	copy.extent =
	{
		m_pImage->GetImageInfo().extent.width,
		m_pImage->GetImageInfo().extent.height,
		m_pImage->GetImageInfo().extent.depth
	};

	copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copy.dstSubresource.baseArrayLayer = 0;
	copy.dstSubresource.layerCount = 1;
	copy.dstSubresource.mipLevel = 0;
	copy.dstOffset = { 0, 0, 0 };

	vkCmdCopyImage(pCmdBuffer->GetDeviceHandle(),
		m_pImage->GetDeviceHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		pImage->GetDeviceHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &copy);

	fbBarrier = {};
	fbBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	fbBarrier.image = m_pImage->GetDeviceHandle();
	fbBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	fbBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	fbBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	fbBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	fbBarrier.subresourceRange =
	{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0, 1,
		0, 1
	};

	inputImgBarrier = {};
	inputImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	inputImgBarrier.image = pImage->GetDeviceHandle();
	inputImgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	inputImgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	inputImgBarrier.newLayout = pImage->GetImageInfo().initialLayout;
	if (inputImgBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		inputImgBarrier.dstAccessMask |= (VK_ACCESS_SHADER_READ_BIT);
	if (inputImgBarrier.newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		inputImgBarrier.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	inputImgBarrier.subresourceRange =
	{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0, 1,
		0, 1
	};

	barriers.clear();
	barriers = { fbBarrier, inputImgBarrier };

	vkCmdPipelineBarrier
	(
		pCmdBuffer->GetDeviceHandle(),
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		0, nullptr,
		0, nullptr,
		barriers.size(), barriers.data()
	);

	pCmdBuffer->EndRecording();
	GlobalGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
}