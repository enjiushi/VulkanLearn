#include "SwapChainImage.h"
#include "GlobalDeviceObjects.h"
#include "SwapChain.h"

bool SwapChainImage::Init(const std::shared_ptr<Device>& pDevice, VkImage rawImageHandle)
{
	if (!Image::Init(pDevice, rawImageHandle))
		return false;

	m_shouldDestoryRawImage = false;

	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	m_info.format = pDevice->GetPhysicalDevice()->GetSurfaceFormat().format;
	m_info.arrayLayers = 1;
	m_info.extent.depth = 1;
	m_info.extent.width = pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
	m_info.extent.height = pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
	m_info.imageType = VK_IMAGE_TYPE_2D;
	m_info.mipLevels = 1;
	m_info.samples = VK_SAMPLE_COUNT_1_BIT;

	//Create image view
	m_viewInfo = {};
	m_viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	m_viewInfo.format = pDevice->GetPhysicalDevice()->GetSurfaceFormat().format;
	m_viewInfo.image = rawImageHandle;
	m_viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	m_viewInfo.subresourceRange.baseArrayLayer = 0;
	m_viewInfo.subresourceRange.layerCount = 1;
	m_viewInfo.subresourceRange.baseMipLevel = 0;
	m_viewInfo.subresourceRange.levelCount = 1;
	m_viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	m_viewInfo.components =
	{
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	CHECK_VK_ERROR(vkCreateImageView(m_pDevice->GetDeviceHandle(), &m_viewInfo, nullptr, &m_view));
	return true;
}

std::vector<std::shared_ptr<SwapChainImage>> SwapChainImage::Create(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SwapChain>& pSwapChain)
{
	std::vector<VkImage> rawImgList;
	uint32_t count;
	CHECK_VK_ERROR((pSwapChain->GetGetSwapchainImagesFuncPtr())(pDevice->GetDeviceHandle(), pSwapChain->GetDeviceHandle(), &count, nullptr));
	rawImgList.resize(count);
	CHECK_VK_ERROR((pSwapChain->GetGetSwapchainImagesFuncPtr())(pDevice->GetDeviceHandle(), pSwapChain->GetDeviceHandle(), &count, rawImgList.data()));

	std::vector<std::shared_ptr<SwapChainImage>> imgList;
	for (uint32_t i = 0; i < count; i++)
	{
		imgList.push_back(Create(pDevice, rawImgList[i]));
	}
	return imgList;
}

std::shared_ptr<SwapChainImage> SwapChainImage::Create(const std::shared_ptr<Device>& pDevice, VkImage rawImageHandle)
{
	std::shared_ptr<SwapChainImage> pImage = std::make_shared<SwapChainImage>();
	if (pImage.get() && pImage->Init(pDevice, rawImageHandle))
		return pImage;
	return nullptr;
}