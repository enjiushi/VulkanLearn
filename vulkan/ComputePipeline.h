#pragma once

#include "DeviceObjectBase.h"
#include "ShaderModule.h"
#include "PipelineBase.h"

class PipelineLayout;

class ComputePipeline : public PipelineBase
{
public:
	~ComputePipeline();

public:
	const VkComputePipelineCreateInfo& GetInfo() const { return m_info; }
	std::shared_ptr<ShaderModule> GetShader() const { return m_pShaderModule; }
	VkPipelineBindPoint GetPipelineBindingPoint() const override { return VK_PIPELINE_BIND_POINT_COMPUTE; }

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
	VkPipeline CreatePipeline() override;

protected:
	VkComputePipelineCreateInfo			m_info;
	VkPipelineShaderStageCreateInfo		m_shaderStageInfo;
	std::shared_ptr<ShaderModule>		m_pShaderModule;
};