#pragma once
#include "../common/RefCounted.h"
#include "vulkan.h"
#include "VulkanInstance.h"
#include "PhysicalDevice.h"
#include <vector>

class VulkanDevice
{
public:
	~VulkanDevice();

	const VkDevice GetDeviceHandle() const { return m_device; }

	bool Init(const std::shared_ptr<VulkanInstance> pInst, const std::shared_ptr<PhysicalDevice> pPhyisicalDevice);

protected:
	VkDevice							m_device;
	std::shared_ptr<PhysicalDevice>	m_pPhysicalDevice;
	std::shared_ptr<VulkanInstance>		m_pVulkanInst;
};