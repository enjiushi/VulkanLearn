#pragma once
#include "../common/RefCounted.h"
#include "vulkan.h"
#include "PhysicalDevice.h"
#include "VulkanDevice.h"
#include <vector>

class SwapChain : public RefCounted
{
public:
	static SwapChain* CreateSwapChain(const PhysicalDevice* pPhyisicalDevice, const VulkanDevice* pDevice);
	~SwapChain();

protected:
	SwapChain() : m_swapchain(0) {}
	bool Init(const PhysicalDevice* pPhyisicalDevice, const VulkanDevice* pDevice);

protected:
	VkSwapchainKHR						m_swapchain;
	AutoPTR<VulkanDevice>				m_pDevice;
	AutoPTR<PhysicalDevice>				m_pPhysicalDevice;

	PFN_vkCreateSwapchainKHR			m_fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR			m_fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR			m_fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR			m_fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR				m_fpQueuePresentKHR;
};