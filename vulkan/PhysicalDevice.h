#pragma once
#include "../common/RefCounted.h"
#include "vulkan.h"
#include "Instance.h"
#include <vector>
#include <memory>

class PhysicalDevice
{
public:
	enum class QueueFamily
	{
		ALL_ROUND,
		COMPUTE,
		TRASFER,
		COUNT
	};

public:
	~PhysicalDevice();

#if defined(_WIN32)
	bool Init(const std::shared_ptr<Instance>& pVulkanInstance, HINSTANCE hInst, HWND hWnd);
#endif

public:
	const VkPhysicalDevice GetDeviceHandle() const { return m_physicalDevice; }
	const VkSurfaceKHR GetSurfaceHandle() const { return m_surface; }
	const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const { return m_physicalDeviceProperties; }
	const VkPhysicalDeviceFeatures& GetPhysicalDeviceFeatures() const { return m_physicalDeviceFeatures; }
	const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const { return m_physicalDeviceMemoryProperties; }
	VkFormatProperties GetPhysicalDeviceFormatProperties(VkFormat format) const;

	const std::vector<VkQueueFamilyProperties>& GetQueueProperties() const { return m_queueProperties; }
	const VkFormat GetDepthStencilFormat() const { return m_depthStencilFormat; }

	uint32_t GetQueueFamilyIndex(QueueFamily queueFamily) const { return m_queueFamilyIndices[(uint32_t)queueFamily]; }

	const VkSurfaceFormatKHR GetSurfaceFormat() const { return m_surfaceFormats[0]; }
	const std::vector<VkPresentModeKHR>& GetPresentModes() const { return m_presentModes; }
	const VkSurfaceCapabilitiesKHR& GetSurfaceCap() const { return m_surfaceCap; }

public:
	static std::shared_ptr<PhysicalDevice> Create(const std::shared_ptr<Instance>& pVulkanInstance, HINSTANCE hInst, HWND hWnd);

private:
	std::shared_ptr<Instance>			m_pVulkanInstance;
	VkPhysicalDevice					m_physicalDevice;
	VkPhysicalDeviceProperties			m_physicalDeviceProperties;
	VkPhysicalDeviceFeatures			m_physicalDeviceFeatures;
	VkPhysicalDeviceMemoryProperties	m_physicalDeviceMemoryProperties;

	std::vector<VkQueueFamilyProperties>	m_queueProperties;
	VkFormat							m_depthStencilFormat;

	uint32_t							m_queueFamilyIndices[(uint32_t)QueueFamily::COUNT];

	//Surface related
	VkSurfaceKHR						m_surface;

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