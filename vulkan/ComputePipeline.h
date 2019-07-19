#pragma once

#include "DeviceObjectBase.h"
#include "ShaderModule.h"

class PipelineLayout;

class ComputePipeline : public DeviceObjectBase<ComputePipeline>
{
	static const uint32_t ENTRY_NAME_LENGTH = 64;

public:
	~ComputePipeline();

public:
	VkPipeline GetDeviceHandle() const { return m_pipeline; }
	const VkComputePipelineCreateInfo& GetInfo() const { return m_info; }
	std::shared_ptr<PipelineLayout> GetPipelineLayout() const { return m_pPipelineLayout; }
	std::shared_ptr<ShaderModule> GetShader() const { return m_pShaderModule; }

public:
	static std::shared_ptr<ComputePipeline> Create
	(
		const std::shared_ptr<Device>& pDevice, 
		const VkComputePipelineCreateInfo& info,
		const std::shared_ptr<ShaderModule>& pShader,
		const std::shared_ptr<PipelineLayout>& pPipelineLayout
	);

private:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<ComputePipeline>& pSelf, const VkComputePipelineCreateInfo& info);

protected:
	VkPipeline							m_pipeline;
	VkComputePipelineCreateInfo			m_info;
	VkPipelineShaderStageCreateInfo		m_shaderStageInfo;
	std::shared_ptr<PipelineLayout>		m_pPipelineLayout;
	std::shared_ptr<ShaderModule>		m_pShaderModule;
};