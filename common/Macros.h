#if defined(_WIN32)
#include <assert.h>
#endif

#include <iostream>

#define EXTENSION_VULKAN_SURFACE "VK_KHR_surface"  
#define EXTENSION_VULKAN_SURFACE_WIN32 "VK_KHR_win32_surface"
#define EXTENSION_VULKAN_VALIDATION_LAYER "VK_LAYER_LUNARG_standard_validation"
#define EXTENSION_VULKAN_DEBUG_REPORT "VK_EXT_debug_report"
#define EXTENSION_VULKAN_SWAPCHAIN "VK_KHR_swapchain"
#define EXTENSION_SHADER_DRAW_PARAMETERS "VK_KHR_shader_draw_parameters"
#define PROJECT_NAME "VulkanLearn"

#define UINT64_MAX       0xffffffffffffffffui64

#if defined(_WIN32)
#if defined(_DEBUG)
#define CHECK_VK_ERROR(vkExpress) { \
	VkResult result = vkExpress; \
	assert(result == VK_SUCCESS); \
	} 
#define RETURN_FALSE_VK_RESULT(vkExpress) { \
	VkResult result = vkExpress;	\
	if (result != VK_SUCCESS)	{	\
		std::cout << "Vulkan call failed, error code:" << result << std::endl;	\
		return false;	\
	}	\
}	
#define ASSERTION(express) assert(express);
#else
#define CHECK_VK_ERROR(vkExpress) vkExpress;
#define RETURN_FALSE_VK_RESULT(vkExpress) vkExpress;
#define CHECK_ERROR(vkExpress) vkExpress;
#define ASSERTION(express) express;
#endif
#endif

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                        \
{                                                                       \
    m_fp##entrypoint = (PFN_vk##entrypoint) vkGetInstanceProcAddr(inst, "vk"#entrypoint); \
    if (m_fp##entrypoint == NULL)                                         \
	{																    \
        exit(1);                                                        \
    }                                                                   \
}

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                           \
{                                                                       \
    m_fp##entrypoint = (PFN_vk##entrypoint) vkGetDeviceProcAddr(dev, "vk"#entrypoint);   \
    if (m_fp##entrypoint == NULL)                                         \
	{																    \
        exit(1);                                                        \
    }                                                                   \
}

#define SAFE_DELETE(x) if (x) { delete x; x = nullptr; }