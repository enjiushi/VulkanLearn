#pragma once

#include "DeviceObjectBase.h"
#include "ShaderModule.h"

class RenderPass;
class PipelineLayout;

class GraphicPipeline : public DeviceObjectBase<GraphicPipeline>
{
	static const uint32_t ENTRY_NAME_LENGTH = 64;
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
	std::shared_ptr<PipelineLayout> GetPipelineLayout() const { return m_pPipelineLayout; }
	std::shared_ptr<RenderPass> GetRenderPass() const { return m_pRenderPass; }
	std::shared_ptr<ShaderModule> GetShader(ShaderModule::ShaderType type) const { return m_shaders[(uint32_t)type]; }

public:
	static std::shared_ptr<GraphicPipeline> Create(const std::shared_ptr<Device>& pDevice, const SimplePipelineStateCreateInfo& info);
	static std::shared_ptr<GraphicPipeline> Create
	(
		const std::shared_ptr<Device>& pDevice, 
		const VkGraphicsPipelineCreateInfo& info,
		const std::vector<std::shared_ptr<ShaderModule>> shaders,
		const std::shared_ptr<RenderPass>& pRenderPass,
		const std::shared_ptr<PipelineLayout>& pPipelineLayout
	);

	static void FillupPipelineCreateInfo
	(
		VkGraphicsPipelineCreateInfo& info,
		const std::vector<std::shared_ptr<ShaderModule>> shaders,
		const std::shared_ptr<RenderPass>& pRenderPass,
		const std::shared_ptr<PipelineLayout>& pPipelineLayout
	);

private:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<GraphicPipeline>& pSelf, const VkGraphicsPipelineCreateInfo& info);

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

	std::shared_ptr<RenderPass>							m_pRenderPass;
	std::shared_ptr<PipelineLayout>						m_pPipelineLayout;
	std::vector<std::shared_ptr<ShaderModule>>			m_shaders;
};