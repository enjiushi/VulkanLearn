#include "SwapChain.h"
#include "../common/Macros.h"

SwapChain::~SwapChain()
{
	if (m_pDevice.get())
		m_fpDestroySwapchainKHR(m_pDevice->GetDeviceHandle(), m_swapchain, nullptr);
}

bool SwapChain::Init(const std::shared_ptr<PhysicalDevice> pPhyisicalDevice, const std::shared_ptr<Device> pDevice)
{
	m_pDevice = pDevice;
	m_pPhysicalDevice = pPhyisicalDevice;

	GET_DEVICE_PROC_ADDR(pDevice->GetDeviceHandle(), CreateSwapchainKHR);
	GET_DEVICE_PROC_ADDR(pDevice->GetDeviceHandle(), DestroySwapchainKHR);
	GET_DEVICE_PROC_ADDR(pDevice->GetDeviceHandle(), GetSwapchainImagesKHR);
	GET_DEVICE_PROC_ADDR(pDevice->GetDeviceHandle(), AcquireNextImageKHR);
	GET_DEVICE_PROC_ADDR(pDevice->GetDeviceHandle(), QueuePresentKHR);

	//Prefer mailbox mode if present, it's the lowest latency non-tearing present  mode
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < m_pPhysicalDevice->GetPresentModes().size(); i++)
	{
		if (m_pPhysicalDevice->GetPresentModes()[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (m_pPhysicalDevice->GetPresentModes()[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	//Don't know what's this
	VkSurfaceTransformFlagsKHR preTransform;
	if (m_pPhysicalDevice->GetSurfaceCap().supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = m_pPhysicalDevice->GetSurfaceCap().currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = pPhyisicalDevice->GetSurfaceHandle();
	swapchainCreateInfo.minImageCount = pPhyisicalDevice->GetSurfaceCap().maxImageCount;
	swapchainCreateInfo.presentMode = swapchainPresentMode;
	swapchainCreateInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapchainCreateInfo.clipped = true;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageExtent.width = pPhyisicalDevice->GetSurfaceCap().currentExtent.width;
	swapchainCreateInfo.imageExtent.height = pPhyisicalDevice->GetSurfaceCap().currentExtent.height;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.imageFormat = pPhyisicalDevice->GetSurfaceFormat().format;
	swapchainCreateInfo.imageColorSpace = pPhyisicalDevice->GetSurfaceFormat().colorSpace;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	RETURN_FALSE_VK_RESULT(m_fpCreateSwapchainKHR(m_pDevice->GetDeviceHandle(), &swapchainCreateInfo, nullptr, &m_swapchain));

	return true;
}