#include "DepthStencilBuffer.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include "Queue.h"
#include "CommandBuffer.h"
#include "ImageView.h"

bool DepthStencilBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<DepthStencilBuffer>& pSelf, const VkImageCreateInfo& info)
{
	if (!Image::Init(pDevice, pSelf, info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	return true;
}

std::shared_ptr<DepthStencilBuffer> DepthStencilBuffer::Create(const std::shared_ptr<Device>& pDevice, VkFormat format, uint32_t width, uint32_t height)
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
	dsCreateInfo.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	dsCreateInfo.mipLevels = 1;
	dsCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	dsCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	std::shared_ptr<DepthStencilBuffer> pDSBuffer = std::make_shared<DepthStencilBuffer>();
	if (pDSBuffer.get() && pDSBuffer->Init(pDevice, pDSBuffer, dsCreateInfo))
	{
		pDSBuffer->m_accessStages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		pDSBuffer->m_accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		return pDSBuffer;
	}
	return nullptr;
}

std::shared_ptr<DepthStencilBuffer> DepthStencilBuffer::Create(const std::shared_ptr<Device>& pDevice)
{
	VkImageCreateInfo dsCreateInfo = {};
	dsCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	dsCreateInfo.format = pDevice->GetPhysicalDevice()->GetDepthStencilFormat();
	dsCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	dsCreateInfo.arrayLayers = 1;
	dsCreateInfo.extent.depth = 1;
	dsCreateInfo.extent.width = pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
	dsCreateInfo.extent.height = pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
	dsCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	dsCreateInfo.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	dsCreateInfo.mipLevels = 1;
	dsCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	dsCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	std::shared_ptr<DepthStencilBuffer> pDSBuffer = std::make_shared<DepthStencilBuffer>();
	if (pDSBuffer.get() && pDSBuffer->Init(pDevice, pDSBuffer, dsCreateInfo))
	{
		pDSBuffer->m_accessStages =  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		pDSBuffer->m_accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		return pDSBuffer;
	}
	return nullptr;
}

std::shared_ptr<DepthStencilBuffer> DepthStencilBuffer::CreateInputAttachment(const std::shared_ptr<Device>& pDevice)
{
	VkImageCreateInfo dsCreateInfo = {};
	dsCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	dsCreateInfo.format = pDevice->GetPhysicalDevice()->GetDepthStencilFormat();
	dsCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	dsCreateInfo.arrayLayers = 1;
	dsCreateInfo.extent.depth = 1;
	dsCreateInfo.extent.width = pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
	dsCreateInfo.extent.height = pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
	dsCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	dsCreateInfo.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	dsCreateInfo.mipLevels = 1;
	dsCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	dsCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	std::shared_ptr<DepthStencilBuffer> pDSBuffer = std::make_shared<DepthStencilBuffer>();
	if (pDSBuffer.get() && pDSBuffer->Init(pDevice, pDSBuffer, dsCreateInfo))
	{
		pDSBuffer->m_accessStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
			| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
			| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		pDSBuffer->m_accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT 
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
			| VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		return pDSBuffer;
	}
	return nullptr;
}

std::shared_ptr<ImageView> DepthStencilBuffer::CreateDefaultImageView() const
{
	//Create depth stencil image view
	VkImageViewCreateInfo imgViewCreateInfo = {};
	imgViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewCreateInfo.image = m_image;
	imgViewCreateInfo.format = m_info.format;
	imgViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	imgViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imgViewCreateInfo.subresourceRange.layerCount = 1;
	imgViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imgViewCreateInfo.subresourceRange.levelCount = 1;

	return ImageView::Create(GetDevice(), imgViewCreateInfo);
}

std::shared_ptr<ImageView> DepthStencilBuffer::CreateDepthSampleImageView() const
{
	VkImageViewCreateInfo imgViewCreateInfo = {};
	imgViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewCreateInfo.image = m_image;
	imgViewCreateInfo.format = m_info.format;
	imgViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	imgViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imgViewCreateInfo.subresourceRange.layerCount = 1;
	imgViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imgViewCreateInfo.subresourceRange.levelCount = 1;

	return ImageView::Create(GetDevice(), imgViewCreateInfo);
}