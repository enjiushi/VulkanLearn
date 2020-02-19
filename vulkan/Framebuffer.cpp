#include "Framebuffer.h"
#include "Image.h"
#include "RenderPass.h"
#include "Image.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Queue.h"
#include "VulkanUtil.h"
#include "ImageView.h"
#include "../Maths/Vector.h"

FrameBuffer::~FrameBuffer()
{
	vkDestroyFramebuffer(GetDevice()->GetDeviceHandle(), m_framebuffer, nullptr);
}

bool FrameBuffer::Init(
	const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<FrameBuffer>& pSelf,
	const std::vector<std::shared_ptr<Image>>& images,
	const std::shared_ptr<Image> pDepthStencilBuffer,
	const std::shared_ptr<RenderPass>& pRenderPass)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_images = images;
	m_pDepthStencilBuffer = pDepthStencilBuffer;
	m_pRenderPass = pRenderPass;

	Vector2ui size = { 0, 0 };

	bool sizeAcquired = false;
	for (auto& pImg : m_images)
	{
		m_imageViews.push_back(pImg->CreateDefaultImageView());
		size = { m_images[0]->GetImageInfo().extent.width, m_images[0]->GetImageInfo().extent.height };
		sizeAcquired = true;
	}
	if (m_pDepthStencilBuffer != nullptr)
	{
		m_imageViews.push_back(m_pDepthStencilBuffer->CreateDefaultImageView());

		if (!sizeAcquired)
			size = { m_pDepthStencilBuffer->GetImageInfo().extent.width, m_pDepthStencilBuffer->GetImageInfo().extent.height };
	}

	std::vector<VkImageView> rawViews;
	for (auto& pImgView : m_imageViews)
		rawViews.push_back(pImgView->GetDeviceHandle());

	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	m_info.attachmentCount = (uint32_t)rawViews.size();
	m_info.pAttachments = rawViews.data();
	m_info.layers = 1;
	m_info.width = size.x;
	m_info.height = size.y;
	m_info.renderPass = m_pRenderPass->GetDeviceHandle();

	RETURN_FALSE_VK_RESULT(vkCreateFramebuffer(GetDevice()->GetDeviceHandle(), &m_info, nullptr, &m_framebuffer));

	return true;
}

std::shared_ptr<FrameBuffer> FrameBuffer::Create(
	const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<Image>& pImage,
	const std::shared_ptr<Image> pDepthStencilBuffer,
	const std::shared_ptr<RenderPass>& pRenderPass)
{
	std::shared_ptr<FrameBuffer> pFramebuffer = std::make_shared<FrameBuffer>();
	if (pFramebuffer.get() && pFramebuffer->Init(pDevice, pFramebuffer, { pImage }, pDepthStencilBuffer, pRenderPass))
		return pFramebuffer;
	return nullptr;
}

std::shared_ptr<FrameBuffer> FrameBuffer::Create(
	const std::shared_ptr<Device>& pDevice,
	const std::vector<std::shared_ptr<Image>>& images,
	const std::shared_ptr<Image> pDepthStencilBuffer,
	const std::shared_ptr<RenderPass>& pRenderPass)
{
	std::shared_ptr<FrameBuffer> pFramebuffer = std::make_shared<FrameBuffer>();
	if (pFramebuffer.get() && pFramebuffer->Init(pDevice, pFramebuffer, images, pDepthStencilBuffer, pRenderPass))
		return pFramebuffer;
	return nullptr;
}

std::shared_ptr<FrameBuffer> FrameBuffer::CreateOffScreenFrameBuffer(
	const std::shared_ptr<Device>& pDevice,
	uint32_t width, uint32_t height,
	const std::shared_ptr<RenderPass>& pRenderPass)
{
	std::shared_ptr<Image> pOffScreenTex = Image::CreateOffscreenTexture2D(pDevice, { width, height }, VK_FORMAT_R16G16B16A16_SFLOAT);
	std::shared_ptr<Image> pDSBuffer = Image::CreateDepthStencilBuffer(pDevice, VK_FORMAT_D32_SFLOAT_S8_UINT, width, height);

	std::shared_ptr<FrameBuffer> pFramebuffer = std::make_shared<FrameBuffer>();
	if (pFramebuffer.get() && pFramebuffer->Init(pDevice, pFramebuffer, { pOffScreenTex }, pDSBuffer, pRenderPass))
		return pFramebuffer;
	return nullptr;
}

void FrameBuffer::ExtractContent(const std::shared_ptr<Image>& pImage, uint32_t index)
{
	ExtractContent(pImage, 0, 1, 0, 1, index);
}

void FrameBuffer::ExtractContent(const std::shared_ptr<Image>& pImage, uint32_t baseMipLevel, uint32_t numMipLevels, uint32_t baseLayer, uint32_t numLayers, uint32_t index)
{
	ExtractContent(pImage, baseMipLevel, numMipLevels, baseLayer, numLayers, m_images[index]->GetImageInfo().extent.width, m_images[index]->GetImageInfo().extent.height, index);
}

void FrameBuffer::ExtractContent(const std::shared_ptr<Image>& pImage, uint32_t baseMipLevel, uint32_t numMipLevels, uint32_t baseLayer, uint32_t numLayers, uint32_t width, uint32_t height, uint32_t index)
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadGraphicPool()->AllocatePrimaryCommandBuffer();
	pCmdBuffer->StartPrimaryRecording();

	VkImageCopy copy = {};

	copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copy.srcSubresource.baseArrayLayer = 0;
	copy.srcSubresource.layerCount = 1;
	copy.srcSubresource.mipLevel = 0;
	copy.srcOffset = { 0, 0, 0 };

	copy.extent =
	{
		width,
		height,
		1
	};

	copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copy.dstSubresource.baseArrayLayer = baseLayer;
	copy.dstSubresource.layerCount = numLayers;
	copy.dstSubresource.mipLevel = baseMipLevel;
	copy.dstOffset = { 0, 0, 0 };

	pCmdBuffer->CopyImage(m_images[index], pImage, { copy } );

	pCmdBuffer->EndPrimaryRecording();
	GlobalGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
}