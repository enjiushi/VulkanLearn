#include "VulkanDevice.h"
#include "../common/Macros.h"
#include <array>

VulkanDevice::~VulkanDevice()
{
	vkDestroyDevice(m_device, nullptr);
}

VulkanDevice* VulkanDevice::CreateVulkanDevice(const VulkanInstance* pInst, const PhysicalDevice* pPhyisicalDevice)
{
	VulkanDevice* pRet = new VulkanDevice;
	if (pRet && pRet->Init(pInst, pPhyisicalDevice))
		return pRet;

	SAFE_DELETE(pRet);
	return pRet;
}

bool VulkanDevice::Init(const VulkanInstance* pInst, const PhysicalDevice* pPhyisicalDevice)
{
	m_pVulkanInst = pInst;
	m_pPhysicalDevice = pPhyisicalDevice;

	std::array<float, 1> queueProperties = { 0.0f };
	VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueFamilyIndex = m_pPhysicalDevice->GetGraphicQueueIndex();
	deviceQueueCreateInfo.queueCount = queueProperties.size();
	deviceQueueCreateInfo.pQueuePriorities = queueProperties.data();

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	std::vector<const char*> extensions = { EXTENSION_VULKAN_SWAPCHAIN };
	deviceCreateInfo.enabledExtensionCount = extensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

	CHECK_VK_ERROR(vkCreateDevice(m_pPhysicalDevice->GetDeviceHandle(), &deviceCreateInfo, nullptr, &m_device));

	return true;
}