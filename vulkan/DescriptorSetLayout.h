#pragma once

#include "DeviceObjectBase.h"

class DescriptorSetLayout : public DeviceObjectBase
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		const std::vector<VkDescriptorSetLayoutBinding>& dsLayoutBinding);

public:
	const std::vector<VkDescriptorSetLayoutBinding>& GetDescriptorSetLayoutBinding() const { return m_descriptorSetLayoutBinding; }
	VkDescriptorSetLayout GetDeviceHandle() const { return m_descriptorSetLayout; }

public:
	static std::shared_ptr<DescriptorSetLayout> Create(const std::shared_ptr<Device>& pDevice,
		const std::vector<VkDescriptorSetLayoutBinding>& dsLayoutBinding);

protected:
	std::vector<VkDescriptorSetLayoutBinding>		m_descriptorSetLayoutBinding;
	VkDescriptorSetLayout							m_descriptorSetLayout;
};