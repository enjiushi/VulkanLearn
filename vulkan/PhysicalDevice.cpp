#include "PhysicalDevice.h"
#include "VulkanInstance.h"
#include "../common/Macros.h"

PhysicalDevice::~PhysicalDevice()
{
}

PhysicalDevice* PhysicalDevice::AcquirePhysicalDevice(const VulkanInstance* pVulkanInstance)
{
	PhysicalDevice* pRet = new PhysicalDevice();
	if (pRet && pRet->Init(pVulkanInstance))
		return pRet;

	SAFE_DELETE(pRet);
	return pRet;
}

bool PhysicalDevice::Init(const VulkanInstance* pVulkanInstance)
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

	return true;
}
