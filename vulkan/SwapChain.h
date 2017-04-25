#pragma once
#include "../common/RefCounted.h"
#include "vulkan.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include <vector>
#include <memory>
#include "DeviceObjectBase.h"

class SwapChain : public DeviceObjectBase
{
public:
	~SwapChain();

	bool Init(const std::shared_ptr<Device>& pDevice) override;

	const VkSwapchainKHR GetDeviceHandle() const { return m_swapchain; }

	PFN_vkAcquireNextImageKHR GetAcquireNextImageFuncPtr() const { return m_fpAcquireNextImageKHR; }
	PFN_vkQueuePresentKHR GetQueuePresentFuncPtr() const { return m_fpQueuePresentKHR; }

public:
	static std::shared_ptr<SwapChain> Create(const std::shared_ptr<Device>& pDevice);

protected:
	VkSwapchainKHR						m_swapchain;

	PFN_vkCreateSwapchainKHR			m_fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR			m_fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR			m_fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR			m_fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR				m_fpQueuePresentKHR;
};