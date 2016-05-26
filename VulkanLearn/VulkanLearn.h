#include "vulkan.h"
#include <vector>
#include <array>

typedef struct _swapchainImg
{
	std::vector<VkImage>				images;
	std::vector<VkImageView>			views;
}SwapchainImg;

typedef struct _depthStencil
{
	VkFormat							format;
	VkDeviceMemory						memory;
	VkImage								image;
	VkImageView							view;
}DepthStencil;

typedef struct _buffer
{
	_buffer() { info = {}; info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; }
	VkDeviceMemory						memory;
	VkBuffer							buffer;
	VkMemoryRequirements				reqs;
	VkBufferCreateInfo					info;
}Buffer;

class VulkanInstance
{
public:
	void InitVulkanInstance();
	void InitPhysicalDevice();
	void InitVulkanDevice();
#if defined(_WIN32)
	void SetupWindow(HINSTANCE hInstance, WNDPROC wndproc);
	void HandleMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
	void InitSurface();
	void InitSwapchain();

	void InitCommandPool();
	void InitSetupCommandBuffer();
	void InitSwapchainImgs();
	void InitDepthStencil();
	void InitRenderpass();
	void InitFrameBuffer();
	void InitVertices();

	void Update();

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
	VkSwapchainKHR						m_swapchain;

	uint32_t							m_graphicQueueIndex;
	uint32_t							m_presentQueueIndex;
	
	std::vector<VkQueueFamilyProperties>	m_queueProperties;

	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR	m_fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR		m_fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR	m_fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR		m_fpGetPhysicalDeviceSurfaceSupportKHR;

	PFN_vkCreateDebugReportCallbackEXT				m_fpCreateDebugReportCallbackEXT;
	PFN_vkDebugReportMessageEXT						m_fpDebugReportMessageEXT;
	PFN_vkDestroyDebugReportCallbackEXT				m_fpDestroyDebugReportCallbackEXT;
	VkDebugReportCallbackEXT						m_debugCallback;

	PFN_vkCreateSwapchainKHR						m_fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR						m_fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR						m_fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR						m_fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR							m_fpQueuePresentKHR;

	VkSurfaceFormatKHR					m_surfaceFormat;

	uint32_t							m_width;
	uint32_t							m_height;

	VkCommandPool						m_commandPool;
	VkCommandBuffer						m_setupCommandBuffer;

	uint32_t							m_swapchainImgCount;
	SwapchainImg						m_swapchainImg;
	DepthStencil						m_depthStencil;

	VkRenderPass						m_renderpass;
	std::vector<VkFramebuffer>			m_framebuffers;

	Buffer								m_vertexBuffer;
	Buffer								m_indexBuffer;
#if defined(_WIN32)
	HINSTANCE							m_hPlatformInst;
	HWND								m_hWindow;
#endif
};

