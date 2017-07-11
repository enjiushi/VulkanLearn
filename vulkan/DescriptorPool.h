#pragma once

#include "DeviceObjectBase.h"

class DescriptorSetLayout;
class DescriptorSet;

class DescriptorPool : public DeviceObjectBase<DescriptorPool>
{
public:
	~DescriptorPool();

	bool Init(const std::shared_ptr<Device>& pDevice, 
		const std::shared_ptr<DescriptorPool>& pSelf,
		const VkDescriptorPoolCreateInfo& info);

public:
	VkDescriptorPoolCreateInfo GetDescriptorSetLayoutBinding() const { return m_descriptorPoolInfo; }
	VkDescriptorPool GetDeviceHandle() const { return m_descriptorPool; }
	static std::shared_ptr<DescriptorSet> AllocateDescriptorSet(const std ::shared_ptr<DescriptorPool>& pDescriptorPool, const std::shared_ptr<DescriptorSetLayout>& pDescriptorSetLayout);

public:
	static std::shared_ptr<DescriptorPool> Create(const std::shared_ptr<Device>& pDevice,
		const VkDescriptorPoolCreateInfo& info);

protected:
	VkDescriptorPoolCreateInfo						m_descriptorPoolInfo;
	std::vector<VkDescriptorPoolSize>				m_descriptorPoolSizes;
	VkDescriptorPool								m_descriptorPool;
};