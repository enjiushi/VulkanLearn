#include "VulkanLearn.h"
#include "Macros.h"
#include <vector>

void VulkanInstance::InitVulkanInstance()
{
	VkApplicationInfo appInfo	= {};
	appInfo.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName	= "VulkanLearn";
	appInfo.apiVersion			= (((1) << 22) | ((0) << 12) | (0));

	std::vector<const char*> extensions = { EXTENSION_VULKAN_SURFACE };
#if defined(_WIN32)
	extensions.push_back( EXTENSION_VULKAN_SURFACE_WIN32 );
#endif
	VkInstanceCreateInfo instCreateInfo = {};
	instCreateInfo.sType				= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instCreateInfo.pApplicationInfo		= &appInfo;
	instCreateInfo.enabledExtensionCount = (int32_t)extensions.size();
	instCreateInfo.ppEnabledExtensionNames = extensions.data();

	CHECK_ERROR(vkCreateInstance(&instCreateInfo, nullptr, &m_vulkanInst));
}
