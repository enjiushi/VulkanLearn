#pragma once
#include "../common/AutoPTR.h"
#include "VulkanBase.h"

#define EXTENSION_VULKAN_SURFACE "VK_KHR_surface"  
#define EXTENSION_VULKAN_SURFACE_WIN32 "VK_KHR_win32_surface"
#define EXTENSION_VULKAN_VALIDATION_LAYER "VK_LAYER_LUNARG_standard_validation"
#define EXTENSION_VULKAN_DEBUG_REPORT "VK_EXT_debug_report"
#define EXTENSION_VULKAN_SWAPCHAIN "VK_KHR_swapchain"

class VulkanInstance : public RefCounted
{
public:
	static VulkanInstance* CreateVulkanInstance(const VkInstanceCreateInfo&);
	~VulkanInstance();

	VkInstance GetDeviceHandle() { return m_vulkanInst; }
	const VkInstance GetDeviceHandle() const { return m_vulkanInst; }

protected:
	VulkanInstance() : m_vulkanInst(0) {}

	bool Init(const VkInstanceCreateInfo&);

protected:
	VkInstance	m_vulkanInst;

	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR	m_fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR		m_fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR	m_fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR		m_fpGetPhysicalDeviceSurfaceSupportKHR;

	PFN_vkCreateDebugReportCallbackEXT				m_fpCreateDebugReportCallbackEXT;
	PFN_vkDebugReportMessageEXT						m_fpDebugReportMessageEXT;
	PFN_vkDestroyDebugReportCallbackEXT				m_fpDestroyDebugReportCallbackEXT;
	VkDebugReportCallbackEXT						m_debugCallback;
};