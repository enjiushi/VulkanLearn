#pragma once

#include "DeviceObjectBase.h"

class DescriptorPool : public DeviceObjectBase
{
public:
	~DescriptorPool();

	bool Init(const std::shared_ptr<Device>& pDevice, 
		const VkDescriptorPoolCreateInfo& info);

public:
	VkDescriptorPoolCreateInfo GetDescriptorSetLayoutBinding() const { return m_descriptorPoolInfo; }
	VkDescriptorPool GetDeviceHandle() const { return m_descriptorPool; }

public:
	static std::shared_ptr<DescriptorPool> Create(const std::shared_ptr<Device>& pDevice,
		const VkDescriptorPoolCreateInfo& info);

protected:
	VkDescriptorPoolCreateInfo						m_descriptorPoolInfo;
	std::vector<VkDescriptorPoolSize>				m_descriptorPoolSizes;
	VkDescriptorPool								m_descriptorPool;
};