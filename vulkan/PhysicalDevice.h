#pragma once
#include "../common/RefCounted.h"
#include "vulkan.h"
#include "VulkanInstance.h"
#include <vector>

class PhysicalDevice : public RefCounted
{
public:
#if defined(_WIN32)
	static PhysicalDevice* AcquirePhysicalDevice(const VulkanInstance* pVulkanInstance, HINSTANCE hInst, HWND hWnd);
#endif
	~PhysicalDevice();

	const VkPhysicalDevice GetDeviceHandle() const { return m_physicalDevice; }
	const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const { return m_physicalDeviceProperties; }
	const VkPhysicalDeviceFeatures& GetPhysicalDeviceFeatures() const { return m_physicalDeviceFeatures; }
	const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const {
		return m_physicalDeviceMemoryProperties;
	}

	const std::vector<VkQueueFamilyProperties>& GetQueueProperties() const { return m_queueProperties; }
	const VkFormat GetDepthStencilFormat() const { return m_depthStencilFormat; }

	const uint32_t GetGraphicQueueIndex() const { return m_graphicQueueIndex; }
	const uint32_t GetPresentQueueIndex() const { return m_presentQueueIndex; }

	const VkSurfaceFormatKHR GetSurfaceFormat() const { return m_surfaceFormats[0]; }

protected:
	PhysicalDevice() : m_physicalDevice(0) {}
#if defined(_WIN32)
	bool Init(const VulkanInstance* pVulkanInstance, HINSTANCE hInst, HWND hWnd);
#endif

protected:
	AutoPTR<VulkanInstance>				m_pVulkanInstance;
	VkPhysicalDevice					m_physicalDevice;
	VkPhysicalDeviceProperties			m_physicalDeviceProperties;
	VkPhysicalDeviceFeatures			m_physicalDeviceFeatures;
	VkPhysicalDeviceMemoryProperties	m_physicalDeviceMemoryProperties;

	std::vector<VkQueueFamilyProperties>	m_queueProperties;
	VkFormat							m_depthStencilFormat;

	uint32_t							m_graphicQueueIndex;

	//Surface related
	VkSurfaceKHR						m_surface;

	uint32_t							m_presentQueueIndex;
	std::vector<VkSurfaceFormatKHR>		m_surfaceFormats;
	std::vector<VkPresentModeKHR>		m_presentModes;
	VkSurfaceCapabilitiesKHR			m_surfaceCap;
	
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR	m_fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR		m_fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR	m_fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR		m_fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkCreateSwapchainKHR						m_fpCreateSwapchainKHR;

#if defined(_WIN32)
	PFN_vkCreateWin32SurfaceKHR						m_fpCreateWin32SurfaceKHR;
#endif
	PFN_vkDestroySurfaceKHR							m_fpDestroySurfaceKHR;

	VkSwapchainKHR						m_swapchain;
};