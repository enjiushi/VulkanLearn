#include "DepthStencilBuffer.h"

bool DepthStencilBuffer::Init(const std::shared_ptr<Device>& pDevice, VkFormat format, uint32_t width, uint32_t height)
{
	VkImageCreateInfo dsCreateInfo = {};
	dsCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	dsCreateInfo.format = format;
	dsCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	dsCreateInfo.arrayLayers = 1;
	dsCreateInfo.extent.depth = 1;
	dsCreateInfo.extent.width = width;
	dsCreateInfo.extent.height = height;
	dsCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	dsCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	dsCreateInfo.mipLevels = 1;
	dsCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	dsCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	if (!Image::Init(pDevice, dsCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	//Create depth stencil image view
	m_viewInfo = {};
	m_viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	m_viewInfo.image = m_image;
	m_viewInfo.format = m_info.format;
	m_viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	m_viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	m_viewInfo.subresourceRange.baseArrayLayer = 0;
	m_viewInfo.subresourceRange.layerCount = 1;
	m_viewInfo.subresourceRange.baseMipLevel = 0;
	m_viewInfo.subresourceRange.levelCount = 1;

	CHECK_VK_ERROR(vkCreateImageView(m_pDevice->GetDeviceHandle(), &m_viewInfo, nullptr, &m_view));

	return true;
}

std::shared_ptr<DepthStencilBuffer> DepthStencilBuffer::Create(const std::shared_ptr<Device>& pDevice, VkFormat format, uint32_t width, uint32_t height)
{
	std::shared_ptr<DepthStencilBuffer> pDSBuffer = std::make_shared<DepthStencilBuffer>();
	if (pDSBuffer.get() && pDSBuffer->Init(pDevice, format, width, height))
		return pDSBuffer;
	return nullptr;
}

std::shared_ptr<DepthStencilBuffer> DepthStencilBuffer::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<DepthStencilBuffer> pDSBuffer = std::make_shared<DepthStencilBuffer>();
	VkFormat format = pDevice->GetPhysicalDevice()->GetDepthStencilFormat();
	uint32_t width = pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
	uint32_t height = pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
	if (pDSBuffer.get() && pDSBuffer->Init(pDevice, format, width, height))
		return pDSBuffer;
	return nullptr;
}