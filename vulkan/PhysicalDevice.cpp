#include "PhysicalDevice.h"
#include "VulkanInstance.h"
#include "../common/Macros.h"

PhysicalDevice::~PhysicalDevice()
{
	m_fpDestroySurfaceKHR(m_pVulkanInstance->GetDeviceHandle(), m_surface, nullptr);
}

PhysicalDevice* PhysicalDevice::AcquirePhysicalDevice(const VulkanInstance* pVulkanInstance, HINSTANCE hInst, HWND hWnd)
{
	PhysicalDevice* pRet = new PhysicalDevice();
	if (pRet && pRet->Init(pVulkanInstance, hInst, hWnd))
		return pRet;

	SAFE_DELETE(pRet);
	return pRet;
}

#if defined(_WIN32)
bool PhysicalDevice::Init(const VulkanInstance* pVulkanInstance, HINSTANCE hInst, HWND hWnd)
#endif
{
	//Get an available physical device
	uint32_t gpuCount = 0;
	vkEnumeratePhysicalDevices(pVulkanInstance->GetDeviceHandle(), &gpuCount, nullptr);
	if (gpuCount == 0)
		return false;

	std::vector<VkPhysicalDevice> physicalDevices;
	physicalDevices.resize(gpuCount);
	vkEnumeratePhysicalDevices(pVulkanInstance->GetDeviceHandle(), &gpuCount, physicalDevices.data());

	//Hard coded, using first physical device
	m_physicalDevice = physicalDevices[0];

	//Get queue properties from physical device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
	if (queueFamilyCount == 0)
		return false;

	m_queueProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueProperties.data());

	//Get physical device properties
	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_physicalDeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProperties);

	//Get depth stencil format
	std::vector<VkFormat> formats =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (uint32_t i = 0; i < formats.size(); i++)
	{
		VkFormatProperties prop;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, formats[i], &prop);
		if (prop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			m_depthStencilFormat = formats[i];
			break;
		}
	}

	m_graphicQueueIndex = -1;
	for (uint32_t i = 0; i < m_queueProperties.size(); i++)
	{
		if (m_queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_graphicQueueIndex = i;
			break;
		}
	}

	ASSERTION(m_graphicQueueIndex != -1);

	GET_INSTANCE_PROC_ADDR(pVulkanInstance->GetDeviceHandle(), GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(pVulkanInstance->GetDeviceHandle(), GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(pVulkanInstance->GetDeviceHandle(), GetPhysicalDeviceSurfacePresentModesKHR);
	GET_INSTANCE_PROC_ADDR(pVulkanInstance->GetDeviceHandle(), GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(pVulkanInstance->GetDeviceHandle(), CreateSwapchainKHR);

#if defined(_WIN32)
	GET_INSTANCE_PROC_ADDR(pVulkanInstance->GetDeviceHandle(), CreateWin32SurfaceKHR);
#endif
	GET_INSTANCE_PROC_ADDR(pVulkanInstance->GetDeviceHandle(), DestroySurfaceKHR);
	//return true;
#if defined(_WIN32)
	VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.hinstance = hInst;
	surfaceInfo.hwnd = hWnd;

	CHECK_VK_ERROR(m_fpCreateWin32SurfaceKHR(pVulkanInstance->GetDeviceHandle(), &surfaceInfo, nullptr, &m_surface));
#endif

	CHECK_VK_ERROR(m_fpGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &m_surfaceCap));

	std::vector<VkBool32> supports;
	supports.resize(m_queueProperties.size());
	for (uint32_t i = 0; i < m_queueProperties.size(); i++)
	{
		CHECK_VK_ERROR(m_fpGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supports[i]));
	}

	m_presentQueueIndex = -1;
	for (uint32_t i = 0; i < m_queueProperties.size(); i++)
	{
		if (supports[i])
		{
			m_presentQueueIndex = i;
			break;
		}
	}

	ASSERTION(m_presentQueueIndex != -1);

	uint32_t formatCount;
	CHECK_VK_ERROR(m_fpGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr));
	m_surfaceFormats.resize(formatCount);
	CHECK_VK_ERROR(m_fpGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, m_surfaceFormats.data()));

	uint32_t presentModeCount = -1;
	CHECK_VK_ERROR(m_fpGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr));
	m_presentModes.resize(presentModeCount);
	CHECK_VK_ERROR(m_fpGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, m_presentModes.data()));

	// Prefer mailbox mode if present, it's the lowest latency non-tearing present  mode
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < presentModeCount; i++)
	{
		if (m_presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (m_presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	//Don't know what's this
	VkSurfaceTransformFlagsKHR preTransform;
	if (m_surfaceCap.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = m_surfaceCap.currentTransform;
	}

	m_pVulkanInstance = pVulkanInstance;
	return true;
}
