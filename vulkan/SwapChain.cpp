#include "SwapChain.h"
#include "../common/Macros.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include "Queue.h"
#include "Semaphore.h"

std::shared_ptr<SwapChain> SwapChain::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<SwapChain> pSwapChain = std::make_shared<SwapChain>();
	if (pSwapChain.get() && pSwapChain->Init(pDevice, pSwapChain))
	{
		pSwapChain->m_swapchainImages = SwapChainImage::Create(pDevice, pSwapChain);
		return pSwapChain;
	}

	return nullptr;
}

SwapChain::~SwapChain()
{
	if (m_pDevice.get())
		m_fpDestroySwapchainKHR(m_pDevice->GetDeviceHandle(), m_swapchain, nullptr);
}

bool SwapChain::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SwapChain>& pSelf)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	GET_DEVICE_PROC_ADDR(pDevice->GetDeviceHandle(), CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(pDevice->GetDeviceHandle(), DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(pDevice->GetDeviceHandle(), GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(pDevice->GetDeviceHandle(), AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(pDevice->GetDeviceHandle(), QueuePresentKHR);

	//Prefer mailbox mode if present, it's the lowest latency non-tearing present  mode
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < m_pDevice->GetPhysicalDevice()->GetPresentModes().size(); i++)
	{
		if (m_pDevice->GetPhysicalDevice()->GetPresentModes()[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (m_pDevice->GetPhysicalDevice()->GetPresentModes()[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	//Don't know what's this
	VkSurfaceTransformFlagsKHR preTransform;
	if (m_pDevice->GetPhysicalDevice()->GetSurfaceCap().supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = m_pDevice->GetPhysicalDevice()->GetSurfaceCap().currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = m_pDevice->GetPhysicalDevice()->GetSurfaceHandle();
	swapchainCreateInfo.minImageCount = m_pDevice->GetPhysicalDevice()->GetSurfaceCap().maxImageCount;
	swapchainCreateInfo.presentMode = swapchainPresentMode;
	swapchainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapchainCreateInfo.clipped = true;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageExtent.width = m_pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
	swapchainCreateInfo.imageExtent.height = m_pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.imageFormat = m_pDevice->GetPhysicalDevice()->GetSurfaceFormat().format;
	swapchainCreateInfo.imageColorSpace = m_pDevice->GetPhysicalDevice()->GetSurfaceFormat().colorSpace;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	RETURN_FALSE_VK_RESULT(m_fpCreateSwapchainKHR(m_pDevice->GetDeviceHandle(), &swapchainCreateInfo, nullptr, &m_swapchain));

	return true;
}

void SwapChain::EnsureSwapChainImageLayout()
{
	for (uint32_t i = 0; i < m_swapchainImages.size(); i++)
	{
		m_swapchainImages[i]->EnsureImageLayout();
	}
}

void SwapChain::AcquireNextImage(const std::shared_ptr<Semaphore>& acquireDone, uint32_t& index) const
{
	CHECK_VK_ERROR(m_fpAcquireNextImageKHR(m_pDevice->GetDeviceHandle(), GetDeviceHandle(), UINT64_MAX, acquireDone->GetDeviceHandle(), nullptr, &index));
}

void SwapChain::QueuePresentImage(const std::shared_ptr<Queue>& pPresentQueue, const std::shared_ptr<Semaphore>& renderDone, uint32_t index) const
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.waitSemaphoreCount = 1;
	auto semaphore = renderDone->GetDeviceHandle();
	presentInfo.pWaitSemaphores = &semaphore;
	presentInfo.pImageIndices = &index;
	CHECK_VK_ERROR(m_fpQueuePresentKHR(pPresentQueue->GetDeviceHandle(), &presentInfo));
}