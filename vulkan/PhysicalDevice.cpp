#include "PhysicalDevice.h"
#include "Instance.h"
#include "../common/Macros.h"

PhysicalDevice::~PhysicalDevice()
{
	if (m_pVulkanInstance.get())
		m_fpDestroySurfaceKHR(m_pVulkanInstance->GetDeviceHandle(), m_surface, nullptr);
}

std::shared_ptr<PhysicalDevice> PhysicalDevice::Create(const std::shared_ptr<Instance>& pVulkanInstance, HINSTANCE hInst, HWND hWnd)
{
	std::shared_ptr<PhysicalDevice> pPhysicalDevice = std::make_shared<PhysicalDevice>();
	if (pPhysicalDevice.get() && pPhysicalDevice->Init(pVulkanInstance, hInst, hWnd))
		return pPhysicalDevice;
	return nullptr;
}

#if defined(_WIN32)
bool PhysicalDevice::Init(const std::shared_ptr<Instance>& pVulkanInstance, HINSTANCE hInst, HWND hWnd)
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

	RETURN_FALSE_VK_RESULT(m_fpCreateWin32SurfaceKHR(pVulkanInstance->GetDeviceHandle(), &surfaceInfo, nullptr, &m_surface));
#endif

	RETURN_FALSE_VK_RESULT(m_fpGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &m_surfaceCap));

	std::vector<VkBool32> supports;
	supports.resize(m_queueProperties.size());
	for (uint32_t i = 0; i < m_queueProperties.size(); i++)
	{
		RETURN_FALSE_VK_RESULT(m_fpGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supports[i]));
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
	RETURN_FALSE_VK_RESULT(m_fpGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr));
	m_surfaceFormats.resize(formatCount);
	RETURN_FALSE_VK_RESULT(m_fpGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, m_surfaceFormats.data()));

	uint32_t presentModeCount = -1;
	RETURN_FALSE_VK_RESULT(m_fpGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr));
	m_presentModes.resize(presentModeCount);
	RETURN_FALSE_VK_RESULT(m_fpGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, m_presentModes.data()));

	m_pVulkanInstance = pVulkanInstance;
	return true;
}

VkFormatProperties PhysicalDevice::GetPhysicalDeviceFormatProperties(VkFormat format) const
{
	VkFormatProperties formatProp = {};
	vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &formatProp);
	return formatProp;
}