#include "Device.h"
#include "../common/Macros.h"
#include <array>

Device::~Device()
{
	vkDestroyDevice(m_device, nullptr);
}

std::shared_ptr<Device> Device::Create(const std::shared_ptr<Instance>& pInstance, const std::shared_ptr<PhysicalDevice> pPhyisicalDevice)
{
	std::shared_ptr<Device> pDevice = std::make_shared<Device>();
	if (pDevice.get() && pDevice->Init(pInstance, pPhyisicalDevice))
		return pDevice;
	return nullptr;
}

bool Device::Init(const std::shared_ptr<Instance>& pInst, const std::shared_ptr<PhysicalDevice>& pPhyisicalDevice)
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

	RETURN_FALSE_VK_RESULT(vkCreateDevice(m_pPhysicalDevice->GetDeviceHandle(), &deviceCreateInfo, nullptr, &m_device));

	return true;
}