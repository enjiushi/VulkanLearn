#include "vulkan.h"
#include <vector>
#include <array>

class VulkanInstance
{
public:
	void InitVulkanInstance();
	void InitPhysicalDevice();
	void InitVulkanDevice();
#if defined(_WIN32)
	void SetupWindow(HINSTANCE hInstance, WNDPROC wndProc);
	void InitSurface();
#endif

public:
	static const uint32_t				WINDOW_WIDTH = 1024;
	static const uint32_t				WINDOW_HEIGHT = 768;

protected:
	VkInstance							m_vulkanInst;
	VkSurfaceKHR						m_surface;

	VkPhysicalDevice					m_physicalDevice;
	VkPhysicalDeviceProperties			m_physicalDeviceProperties;
	VkPhysicalDeviceFeatures			m_physicalDeviceFeatures;
	VkPhysicalDeviceMemoryProperties	m_physicalDeviceMemoryProperties;

	VkDevice							m_device;

	uint32_t							m_graphicQueueIndex;
	uint32_t							m_presentQueueIndex;
	
	std::vector<VkQueueFamilyProperties>	m_queueProperties;

	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR	m_fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR		m_fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR	m_fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR		m_fpGetPhysicalDeviceSurfaceSupportKHR;

	PFN_vkCreateSwapchainKHR						m_fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR						m_fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR						m_fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR						m_fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR							m_fpQueuePresentKHR;

	VkSurfaceFormatKHR					m_surfaceFormat;

#if defined(_WIN32)
	HINSTANCE							m_hPlatformInst;
	HWND								m_hWindow;
#endif
};

