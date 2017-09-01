#include "GraphicPipeline.h"
#include "PipelineLayout.h"
#include "RenderPass.h"
#include "ShaderModule.h"
#include <fstream>

GraphicPipeline::~GraphicPipeline()
{
	vkDestroyPipeline(GetDevice()->GetDeviceHandle(), m_pipeline, nullptr);

	for (uint32_t i = 0; i < m_shaderStageInfo.size(); i++)
		delete[] m_shaderStageInfo[i].pName;
}

bool GraphicPipeline::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<GraphicPipeline>& pSelf, const VkGraphicsPipelineCreateInfo& info)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_info = info;
	
	for (uint32_t i = 0; i < m_info.pColorBlendState->attachmentCount; i++)
		m_blendStatesInfo.push_back(m_info.pColorBlendState->pAttachments[i]);

	m_blendCreateInfo = *m_info.pColorBlendState;
	m_blendCreateInfo.pAttachments = m_blendStatesInfo.data();
	m_info.pColorBlendState = &m_blendCreateInfo;

	m_depthStencilCreateInfo = *m_info.pDepthStencilState;
	m_info.pDepthStencilState = &m_depthStencilCreateInfo;

	m_assemblyCreateInfo = *m_info.pInputAssemblyState;
	m_info.pInputAssemblyState = &m_assemblyCreateInfo;

	m_multiSampleCreateInfo = *m_info.pMultisampleState;
	m_info.pMultisampleState = &m_multiSampleCreateInfo;

	m_rasterizerCreateInfo = *m_info.pRasterizationState;
	m_info.pRasterizationState = &m_rasterizerCreateInfo;

	// Force viewport & scissor state to be dynamic
	m_viewportStateCreateInfo = *m_info.pViewportState;
	m_viewportStateCreateInfo.viewportCount = 1;
	m_viewportStateCreateInfo.pScissors = nullptr;
	m_viewportStateCreateInfo.scissorCount = 1;
	m_viewportStateCreateInfo.pViewports = nullptr;

	m_dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	m_dynamicStatesCreateInfo = *m_info.pDynamicState;
	m_dynamicStatesCreateInfo.dynamicStateCount = m_dynamicStates.size();
	m_dynamicStatesCreateInfo.pDynamicStates = m_dynamicStates.data();
	m_info.pDynamicState = &m_dynamicStatesCreateInfo;

	m_shaderStageInfo.resize(m_info.stageCount);
	for (uint32_t i = 0; i < m_info.stageCount; i++)
		m_shaderStageInfo[i] = m_info.pStages[i];
	m_info.pStages = m_shaderStageInfo.data();

	m_vertexBindingsInfo.resize(m_info.pVertexInputState->vertexBindingDescriptionCount);
	for (uint32_t i = 0; i < m_info.pVertexInputState->vertexBindingDescriptionCount; i++)
		m_vertexBindingsInfo[i] = m_info.pVertexInputState->pVertexBindingDescriptions[i];

	m_vertexAttributesInfo.resize(info.pVertexInputState->vertexAttributeDescriptionCount);
	for (uint32_t i = 0; i < info.pVertexInputState->vertexAttributeDescriptionCount; i++)
		m_vertexAttributesInfo[i] = info.pVertexInputState->pVertexAttributeDescriptions[i];

	m_vertexInputCreateInfo = *info.pVertexInputState;
	m_vertexInputCreateInfo.pVertexBindingDescriptions = m_vertexBindingsInfo.data();
	m_vertexInputCreateInfo.pVertexAttributeDescriptions = m_vertexAttributesInfo.data();
	m_info.pVertexInputState = &m_vertexInputCreateInfo;

	CHECK_VK_ERROR(vkCreateGraphicsPipelines(m_pDevice->GetDeviceHandle(), 0, 1, &m_info, nullptr, &m_pipeline));

	return true;
}

std::shared_ptr<GraphicPipeline> GraphicPipeline::Create
(
	const std::shared_ptr<Device>& pDevice,
	const VkGraphicsPipelineCreateInfo& info,
	const std::vector<std::shared_ptr<ShaderModule>> shaders,
	const std::shared_ptr<RenderPass>& pRenderPass,
	const std::shared_ptr<PipelineLayout>& pPipelineLayout
)
{
	std::shared_ptr<GraphicPipeline> pPipeline = std::make_shared<GraphicPipeline>();

	pPipeline->m_shaders.assign(shaders.begin(), shaders.end());
	pPipeline->m_pRenderPass = pRenderPass;
	pPipeline->m_pPipelineLayout = pPipelineLayout;

	VkGraphicsPipelineCreateInfo createInfo = info;
	FillupPipelineCreateInfo(createInfo, pPipeline->m_shaders, pRenderPass, pPipelineLayout);

	if (pPipeline.get() && pPipeline->Init(pDevice, pPipeline, createInfo))
		return pPipeline;
	return nullptr;
}

std::shared_ptr<GraphicPipeline> GraphicPipeline::Create(const std::shared_ptr<Device>& pDevice, const SimplePipelineStateCreateInfo& info)
{
	std::shared_ptr<GraphicPipeline> pPipeline = std::make_shared<GraphicPipeline>();

	pPipeline->m_shaders.resize((uint32_t)ShaderModule::ShaderTypeCount);
	pPipeline->m_shaders[(uint32_t)ShaderModule::ShaderTypeVertex] = info.pVertShader;
	pPipeline->m_shaders[(uint32_t)ShaderModule::ShaderTypeFragment] = info.pFragShader;

	pPipeline->m_pRenderPass		= info.pRenderPass;
	pPipeline->m_pPipelineLayout	= info.pPipelineLayout;

	VkGraphicsPipelineCreateInfo createInfo = {};

	std::vector<VkPipelineColorBlendAttachmentState> blendStatesInfo =
	{
		{
			VK_TRUE,							// blend enabled

			VK_BLEND_FACTOR_SRC_ALPHA,			// src color blend factor
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,// dst color blend factor
			VK_BLEND_OP_ADD,					// color blend op

			VK_BLEND_FACTOR_ONE,				// src alpha blend factor
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,// dst alpha blend factor
			VK_BLEND_OP_ADD,					// alpha blend factor

			0xf,								// color mask
		},
	};

	VkPipelineColorBlendStateCreateInfo blendCreateInfo = {};
	blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendCreateInfo.logicOpEnable = VK_FALSE;
	blendCreateInfo.attachmentCount = 1;
	blendCreateInfo.pAttachments = blendStatesInfo.data();

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	VkPipelineInputAssemblyStateCreateInfo assemblyCreateInfo = {};
	assemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineMultisampleStateCreateInfo multiSampleCreateInfo = {};
	multiSampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerCreateInfo.lineWidth = 1.0f;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pScissors = nullptr;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pViewports = nullptr;

	std::vector<VkDynamicState>	 dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStatesCreateInfo = {};
	dynamicStatesCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStatesCreateInfo.dynamicStateCount = dynamicStates.size();
	dynamicStatesCreateInfo.pDynamicStates = dynamicStates.data();

	char* pVertEntryName = new char[ENTRY_NAME_LENGTH];
	char* pFragEntryName = new char[ENTRY_NAME_LENGTH];
	strcpy(pVertEntryName, info.pVertShader->GetEntryName().c_str());
	strcpy(pFragEntryName, info.pFragShader->GetEntryName().c_str());

	std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfo(2);
	shaderStageInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo[0].stage = info.pVertShader->GetShaderStage();
	shaderStageInfo[0].module = info.pVertShader->GetDeviceHandle();
	shaderStageInfo[0].pName = pVertEntryName;
	shaderStageInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo[1].stage = info.pFragShader->GetShaderStage();
	shaderStageInfo[1].module = info.pFragShader->GetDeviceHandle();
	shaderStageInfo[1].pName = pFragEntryName;
	createInfo.stageCount = shaderStageInfo.size();
	createInfo.pStages = shaderStageInfo.data();

	std::vector<VkVertexInputBindingDescription> vertexBindingsInfo(info.vertexBindingsInfo.size());
	for (uint32_t i = 0; i < info.vertexBindingsInfo.size(); i++)
		vertexBindingsInfo[i] = info.vertexBindingsInfo[i];

	std::vector<VkVertexInputAttributeDescription> vertexAttributesInfo(info.vertexAttributesInfo.size());
	for (uint32_t i = 0; i < info.vertexAttributesInfo.size(); i++)
		vertexAttributesInfo[i] = info.vertexAttributesInfo[i];

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = vertexBindingsInfo.size();
	vertexInputCreateInfo.pVertexBindingDescriptions = vertexBindingsInfo.data();
	vertexInputCreateInfo.vertexAttributeDescriptionCount = vertexAttributesInfo.size();
	vertexInputCreateInfo.pVertexAttributeDescriptions = vertexAttributesInfo.data();

	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.pColorBlendState = &blendCreateInfo;
	createInfo.pDepthStencilState = &depthStencilCreateInfo;
	createInfo.pInputAssemblyState = &assemblyCreateInfo;
	createInfo.pMultisampleState = &multiSampleCreateInfo;
	createInfo.pRasterizationState = &rasterizerCreateInfo;
	createInfo.pViewportState = &viewportStateCreateInfo;
	createInfo.pDynamicState = &dynamicStatesCreateInfo;
	createInfo.renderPass = info.pRenderPass->GetDeviceHandle();
	createInfo.layout = info.pPipelineLayout->GetDeviceHandle();
	createInfo.pVertexInputState = &vertexInputCreateInfo;

	if (pPipeline.get() && pPipeline->Init(pDevice, pPipeline, createInfo))
		return pPipeline;
	return nullptr;
}

void GraphicPipeline::FillupPipelineCreateInfo
(
	VkGraphicsPipelineCreateInfo& info,
	const std::vector<std::shared_ptr<ShaderModule>> shaders,
	const std::shared_ptr<RenderPass>& pRenderPass,
	const std::shared_ptr<PipelineLayout>& pPipelineLayout
)
{
	info.renderPass = pRenderPass->GetDeviceHandle();
	info.layout = pPipelineLayout->GetDeviceHandle();

	std::vector<VkPipelineShaderStageCreateInfo> stages(shaders.size());
	for (uint32_t i = 0; i < stages.size(); i++)
	{
		stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[i].stage = shaders[i]->GetShaderStage();
		stages[i].module = shaders[i]->GetDeviceHandle();

		char* pEntryName = new char[ENTRY_NAME_LENGTH];
		strcpy(pEntryName, shaders[i]->GetEntryName().c_str());
		stages[i].pName = pEntryName;
	}
	info.stageCount = stages.size();
	info.pStages = stages.data();
}