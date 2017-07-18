#include "Instance.h"
#include "../common/Macros.h"

#ifdef _DEBUG
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
#endif

Instance::~Instance()
{
	if (m_vulkanInst)
	{
#ifdef _DEBUG
		m_fpDestroyDebugReportCallbackEXT(m_vulkanInst, m_debugCallback, nullptr);
#endif
		vkDestroyInstance(m_vulkanInst, nullptr);
	}
}

std::shared_ptr<Instance> Instance::Create(const VkInstanceCreateInfo& info)
{
	std::shared_ptr<Instance> pInstance = std::make_shared<Instance>();
	if (pInstance.get() && pInstance->Init(info))
		return pInstance;
	return nullptr;
}

bool Instance::Init(const VkInstanceCreateInfo& info)
{
	RETURN_FALSE_VK_RESULT(vkCreateInstance(&info, nullptr, &m_vulkanInst));

#ifdef _DEBUG
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
#endif

	return true;
}