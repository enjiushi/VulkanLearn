#pragma once

#include "DeviceObjectBase.h"

class RenderPass;
class PipelineLayout;
class ShaderModule;

class GraphicPipeline : public DeviceObjectBase
{
public:
	// Simple pipeline state, nearly all are pre-defined
	typedef struct _SimplePipelineStateCreateInfo
	{
		std::shared_ptr<RenderPass>						pRenderPass;
		std::shared_ptr<PipelineLayout>					pPipelineLayout;
		std::shared_ptr<ShaderModule>					pVertShader;
		std::shared_ptr<ShaderModule>					pFragShader;
		std::vector<VkVertexInputBindingDescription>	vertexBindingsInfo;
		std::vector<VkVertexInputAttributeDescription>	vertexAttributesInfo;
	}SimplePipelineStateCreateInfo;

public:
	~GraphicPipeline();

public:
	VkPipeline GetDeviceHandle() const { return m_pipeline; }

public:
	static std::shared_ptr<GraphicPipeline> Create(const std::shared_ptr<Device>& pDevice, const SimplePipelineStateCreateInfo& info);

private:
	bool Init(const std::shared_ptr<Device>& pDevice);

protected:
	VkPipeline						m_pipeline;

	std::vector<VkPipelineColorBlendAttachmentState>	m_blendStatesInfo;
	std::vector<VkDynamicState>							m_dynamicStates;
	std::vector<VkVertexInputBindingDescription>		m_vertexBindingsInfo;
	std::vector<VkVertexInputAttributeDescription>		m_vertexAttributesInfo;
	std::vector<VkPipelineShaderStageCreateInfo>		m_shaderStageInfo;

	VkPipelineColorBlendStateCreateInfo					m_blendCreateInfo;
	VkPipelineDepthStencilStateCreateInfo				m_depthStencilCreateInfo;
	VkPipelineInputAssemblyStateCreateInfo				m_assemblyCreateInfo;
	VkPipelineMultisampleStateCreateInfo				m_multiSampleCreateInfo;
	VkPipelineRasterizationStateCreateInfo				m_rasterizerCreateInfo;
	VkPipelineViewportStateCreateInfo					m_viewportStateCreateInfo;
	VkPipelineDynamicStateCreateInfo					m_dynamicStatesCreateInfo;
	VkPipelineVertexInputStateCreateInfo				m_vertexInputCreateInfo;
	VkGraphicsPipelineCreateInfo						m_info;

	std::shared_ptr<RenderPass>		m_pRenderPass;
	std::shared_ptr<PipelineLayout> m_pPipelineLayout;
	std::shared_ptr<ShaderModule>	m_vertShader;
	std::shared_ptr<ShaderModule>	m_fragShader;
};