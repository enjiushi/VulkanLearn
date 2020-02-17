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

std::shared_ptr<ImageView> DepthStencilBuffer::CreateDefaultImageView() const
{
	//Create depth stencil image view
	VkImageViewCreateInfo imgViewCreateInfo = {};
	imgViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imgViewCreateInfo.image = m_image;
	imgViewCreateInfo.format = m_info.format;
	imgViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imgViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (m_info.format == VK_FORMAT_D16_UNORM_S8_UINT || m_info.format == VK_FORMAT_D24_UNORM_S8_UINT || m_info.format == VK_FORMAT_D32_SFLOAT_S8_UINT)
		imgViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
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