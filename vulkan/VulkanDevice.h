#pragma once
#include "../common/RefCounted.h"
#include "vulkan.h"
#include "VulkanInstance.h"
#include "PhysicalDevice.h"
#include <vector>

class VulkanDevice : public RefCounted
{
public:
	static VulkanDevice* CreateVulkanDevice(const VulkanInstance* pInst, const PhysicalDevice* pPhyisicalDevice);
	~VulkanDevice();

	const VkDevice GetDeviceHandle() const { return m_device; }

protected:
	VulkanDevice() : m_device(0) {}
	bool Init(const VulkanInstance* pInst, const PhysicalDevice* pPhyisicalDevice);

protected:
	VkDevice							m_device;
	AutoPTR<PhysicalDevice>				m_pPhysicalDevice;
	AutoPTR<VulkanInstance>				m_pVulkanInst;
};