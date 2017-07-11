#pragma once

#include "DeviceObjectBase.h"

class DescriptorSetLayout;

typedef std::vector<std::shared_ptr<DescriptorSetLayout>> DescriptorSetLayoutList;

class PipelineLayout : public DeviceObjectBase<PipelineLayout>
{
public:
	~PipelineLayout();

	bool Init(const std::shared_ptr<Device>& pDevice,
		const std::shared_ptr<PipelineLayout>& pSelf,
		const DescriptorSetLayoutList& descriptorSetLayoutList);

public:
	VkPipelineLayout GetDeviceHandle() const { return m_pipelineLayout; }
	const DescriptorSetLayoutList GetDescriptorSetLayout() const { return m_descriptorSetLayoutList; }
	const std::vector<VkDescriptorSetLayout> GetDescriptorSetLayoutDeviceHandleList();

public:
	static std::shared_ptr<PipelineLayout> Create(const std::shared_ptr<Device>& pDevice,
		const DescriptorSetLayoutList& descriptorSetLayoutList);

protected:
	DescriptorSetLayoutList						m_descriptorSetLayoutList;
	VkPipelineLayout							m_pipelineLayout;
};