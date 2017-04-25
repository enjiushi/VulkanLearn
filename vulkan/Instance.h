#pragma once
#include <memory>
#include "vulkan.h"
#include "../common/Macros.h"

#define EXTENSION_VULKAN_SURFACE "VK_KHR_surface"  
#define EXTENSION_VULKAN_SURFACE_WIN32 "VK_KHR_win32_surface"
#define EXTENSION_VULKAN_VALIDATION_LAYER "VK_LAYER_LUNARG_standard_validation"
#define EXTENSION_VULKAN_DEBUG_REPORT "VK_EXT_debug_report"
#define EXTENSION_VULKAN_SWAPCHAIN "VK_KHR_swapchain"

class Instance
{
public:
	~Instance();

	VkInstance GetDeviceHandle() { return m_vulkanInst; }
	const VkInstance GetDeviceHandle() const { return m_vulkanInst; }

	bool Init(const VkInstanceCreateInfo&);

public:
	static std::shared_ptr<Instance> Create(const VkInstanceCreateInfo&);

private:
	VkInstance	m_vulkanInst;

	PFN_vkCreateDebugReportCallbackEXT				m_fpCreateDebugReportCallbackEXT;
	PFN_vkDebugReportMessageEXT						m_fpDebugReportMessageEXT;
	PFN_vkDestroyDebugReportCallbackEXT				m_fpDestroyDebugReportCallbackEXT;
	VkDebugReportCallbackEXT						m_debugCallback;
};