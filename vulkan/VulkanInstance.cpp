#include "VulkanInstance.h"
#include "../common/Macros.h"

VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
	int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) 
{
#if defined(_WIN32)
	OutputDebugStringA(pLayerPrefix);
	OutputDebugStringA(" ");
	OutputDebugStringA(pMessage);
	OutputDebugStringA("\n");
#endif
	return VK_FALSE;
}

VulkanInstance::~VulkanInstance()
{
	if (m_vulkanInst)
		vkDestroyInstance(m_vulkanInst, nullptr);
}

bool VulkanInstance::Init(const VkInstanceCreateInfo& info)
{
	RETURN_FALSE_VK_RESULT(vkCreateInstance(&info, nullptr, &m_vulkanInst));

	GET_INSTANCE_PROC_ADDR(m_vulkanInst, CreateDebugReportCallbackEXT);
	GET_INSTANCE_PROC_ADDR(m_vulkanInst, DebugReportMessageEXT);
	GET_INSTANCE_PROC_ADDR(m_vulkanInst, DestroyDebugReportCallbackEXT);

	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackCreateInfo.pfnCallback = &MyDebugReportCallback;
	callbackCreateInfo.pUserData = NULL;

	RETURN_FALSE_VK_RESULT(m_fpCreateDebugReportCallbackEXT(m_vulkanInst, &callbackCreateInfo, NULL, &m_debugCallback));

	return true;
}