#include "SwapChain.h"
#include "../common/Macros.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include "Queue.h"

std::shared_ptr<SwapChain> SwapChain::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<SwapChain> pSwapChain = std::make_shared<SwapChain>();
	if (pSwapChain.get() && pSwapChain->Init(pDevice))
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

bool SwapChain::Init(const std::shared_ptr<Device>& pDevice)
{
	if (!DeviceObjectBase::Init(pDevice))
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
	// FIXME: Use native device objects here for now
	VkCommandBuffer cmdBuffer = GlobalDeviceObjects::GetInstance()->GetMainThreadCmdPool()->AllocateCommandBuffer();
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	for (uint32_t i = 0; i < m_swapchainImages.size(); i++)
	{
		//Change image layout
		VkImageMemoryBarrier imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.srcAccessMask = 0;
		imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.image = m_swapchainImages[i]->GetDeviceHandle();
		imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrier.subresourceRange.baseMipLevel = 0;
		imageBarrier.subresourceRange.levelCount = 1;
		imageBarrier.subresourceRange.baseArrayLayer = 0;
		imageBarrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(cmdBuffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageBarrier);
	}

	CHECK_VK_ERROR(vkEndCommandBuffer(cmdBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	CHECK_VK_ERROR(vkQueueSubmit(GlobalDeviceObjects::GetInstance()->GetGraphicQueue()->GetDeviceHandle(), 1, &submitInfo, nullptr));
	vkQueueWaitIdle(GlobalDeviceObjects::GetInstance()->GetGraphicQueue()->GetDeviceHandle());

	vkFreeCommandBuffers(m_pDevice->GetDeviceHandle(), GlobalDeviceObjects::GetInstance()->GetMainThreadCmdPool()->GetDeviceHandle(), 1, &cmdBuffer);
}