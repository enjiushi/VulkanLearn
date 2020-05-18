#include "Device.h"
#include "Queue.h"
#include "../common/Macros.h"
#include <array>
#include <set>

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

	// One queue could be of multiple usages
	// Using set to get rid of redundent queue indices
	std::set<uint32_t> queueIndexSet;
	queueIndexSet.insert(m_pPhysicalDevice->GetQueueFamilyIndex(PhysicalDevice::QueueFamily::ALL_ROUND));
	queueIndexSet.insert(m_pPhysicalDevice->GetQueueFamilyIndex(PhysicalDevice::QueueFamily::COMPUTE));
	queueIndexSet.insert(m_pPhysicalDevice->GetQueueFamilyIndex(PhysicalDevice::QueueFamily::TRASFER));

	std::array<float, 1> queueProperties = { 0.0f };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	for (auto queueIndex : queueIndexSet)
	{
		VkDeviceQueueCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.queueFamilyIndex = queueIndex;
		info.queueCount = (uint32_t)queueProperties.size();
		info.pQueuePriorities = queueProperties.data();
		queueCreateInfos.push_back(info);
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	std::vector<const char*> extensions = { EXTENSION_VULKAN_SWAPCHAIN, EXTENSION_SHADER_DRAW_PARAMETERS, EXTENSION_VULKAN_DRAW_INDIRECT_COUNT };
	deviceCreateInfo.enabledExtensionCount = (uint32_t)extensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = extensions.data();

	VkPhysicalDeviceFeatures enabledFeatures = {};
	enabledFeatures.drawIndirectFirstInstance = 1;
	enabledFeatures.multiDrawIndirect = 1;
	enabledFeatures.fullDrawIndexUint32 = 1;
	enabledFeatures.vertexPipelineStoresAndAtomics = 1;
	enabledFeatures.fragmentStoresAndAtomics = 1;
	enabledFeatures.depthBiasClamp = 1;
	deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

	RETURN_FALSE_VK_RESULT(vkCreateDevice(m_pPhysicalDevice->GetDeviceHandle(), &deviceCreateInfo, nullptr, &m_device));

	GET_DEVICE_PROC_ADDR(m_device, CmdDrawIndexedIndirectCountKHR);

	return true;
}