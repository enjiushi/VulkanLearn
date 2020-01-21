#pragma once

#include "DeviceObjectBase.h"
#include "ShaderModule.h"

class RenderPass;
class PipelineLayout;

class PipelineBase : public DeviceObjectBase<PipelineBase>
{
protected:
	static const uint32_t ENTRY_NAME_LENGTH = 64;

public:
	virtual ~PipelineBase();

public:
	VkPipeline GetDeviceHandle() const { return m_pipeline; }
	std::shared_ptr<PipelineLayout> GetPipelineLayout() const { return m_pPipelineLayout; }
	virtual VkPipelineBindPoint GetPipelineBindingPoint() const = 0;
	virtual uint32_t GetSubpassIndex() const { return 0; }

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		const std::shared_ptr<PipelineBase>& pSelf,
		const std::shared_ptr<PipelineLayout>& pPipelineLayout);

	virtual VkPipeline CreatePipeline() = 0;

protected:
	VkPipeline							m_pipeline;
	std::shared_ptr<PipelineLayout>		m_pPipelineLayout;
	VkPipelineBindPoint					m_pipelineBindingPoint;
};