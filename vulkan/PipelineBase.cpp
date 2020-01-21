#include "PipelineBase.h"
#include "PipelineLayout.h"
#include "ShaderModule.h"
#include <fstream>

PipelineBase::~PipelineBase()
{
	vkDestroyPipeline(GetDevice()->GetDeviceHandle(), m_pipeline, nullptr);
}

bool PipelineBase::Init(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<PipelineBase>& pSelf,
	const std::shared_ptr<PipelineLayout>& pPipelineLayout)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_pPipelineLayout = pPipelineLayout;
	m_pipeline = CreatePipeline();

	return true;
}