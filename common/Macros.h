#if defined(_WIN32)
#include <assert.h>
#endif

#define EXTENSION_VULKAN_SURFACE "VK_KHR_surface"  
#define EXTENSION_VULKAN_SURFACE_WIN32 "VK_KHR_win32_surface"

#if defined(_WIN32)
#if defined(_DEBUG)
#define CHECK_ERROR(vkExpress) { \
	VkResult result = vkExpress; \
	assert(result == VK_SUCCESS); \
	} 
#else
#define CHECK_ERROR(vkExpress) vkExpress;
#endif
#endif