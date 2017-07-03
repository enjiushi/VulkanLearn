#include "GraphicPipeline.h"
#include "PipelineLayout.h"
#include "RenderPass.h"
#include "ShaderModule.h"
#include <fstream>

GraphicPipeline::~GraphicPipeline()
{
	vkDestroyPipeline(GetDevice()->GetDeviceHandle(), m_pipeline, nullptr);
}

bool GraphicPipeline::Init(const std::shared_ptr<Device>& pDevice)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	CHECK_VK_ERROR(vkCreateGraphicsPipelines(m_pDevice->GetDeviceHandle(), 0, 1, &m_info, nullptr, &m_pipeline));

	return true;
}

std::shared_ptr<GraphicPipeline> GraphicPipeline::Create(const std::shared_ptr<Device>& pDevice, const SimplePipelineStateCreateInfo& info)
{
	std::shared_ptr<GraphicPipeline> pPipeline = std::make_shared<GraphicPipeline>();

	pPipeline->m_info = {};

	pPipeline->m_blendStatesInfo =
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

	pPipeline->m_blendCreateInfo = {};
	pPipeline->m_blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pPipeline->m_blendCreateInfo.logicOpEnable = VK_FALSE;
	pPipeline->m_blendCreateInfo.attachmentCount = 1;
	pPipeline->m_blendCreateInfo.pAttachments = pPipeline->m_blendStatesInfo.data();

	pPipeline->m_depthStencilCreateInfo = {};
	pPipeline->m_depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pPipeline->m_depthStencilCreateInfo.depthTestEnable = VK_TRUE;
	pPipeline->m_depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
	pPipeline->m_depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	pPipeline->m_assemblyCreateInfo = {};
	pPipeline->m_assemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pPipeline->m_assemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	pPipeline->m_multiSampleCreateInfo = {};
	pPipeline->m_multiSampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pPipeline->m_multiSampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	pPipeline->m_rasterizerCreateInfo = {};
	pPipeline->m_rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pPipeline->m_rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	pPipeline->m_rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	pPipeline->m_rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pPipeline->m_rasterizerCreateInfo.lineWidth = 1.0f;
	pPipeline->m_rasterizerCreateInfo.depthClampEnable = VK_FALSE;
	pPipeline->m_rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	pPipeline->m_rasterizerCreateInfo.depthBiasEnable = VK_FALSE;

	pPipeline->m_viewportStateCreateInfo = {};
	pPipeline->m_viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pPipeline->m_viewportStateCreateInfo.viewportCount = 1;
	pPipeline->m_viewportStateCreateInfo.pScissors = nullptr;
	pPipeline->m_viewportStateCreateInfo.scissorCount = 1;
	pPipeline->m_viewportStateCreateInfo.pViewports = nullptr;

	pPipeline->m_dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	pPipeline->m_dynamicStatesCreateInfo = {};
	pPipeline->m_dynamicStatesCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pPipeline->m_dynamicStatesCreateInfo.dynamicStateCount = pPipeline->m_dynamicStates.size();
	pPipeline->m_dynamicStatesCreateInfo.pDynamicStates = pPipeline->m_dynamicStates.data();

	pPipeline->m_shaderStageInfo.resize(2);
	pPipeline->m_shaderStageInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pPipeline->m_shaderStageInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	pPipeline->m_shaderStageInfo[0].pName = "main";
	pPipeline->m_shaderStageInfo[0].module = info.pVertShader->GetDeviceHandle();
	pPipeline->m_shaderStageInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pPipeline->m_shaderStageInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pPipeline->m_shaderStageInfo[1].pName = "main";
	pPipeline->m_shaderStageInfo[1].module = info.pFragShader->GetDeviceHandle();
	pPipeline->m_info.stageCount = pPipeline->m_shaderStageInfo.size();
	pPipeline->m_info.pStages = pPipeline->m_shaderStageInfo.data();

	pPipeline->m_vertexBindingsInfo.resize(info.vertexBindingsInfo.size());
	for (uint32_t i = 0; i < info.vertexBindingsInfo.size(); i++)
		pPipeline->m_vertexBindingsInfo[i] = info.vertexBindingsInfo[i];

	pPipeline->m_vertexAttributesInfo.resize(info.vertexAttributesInfo.size());
	for (uint32_t i = 0; i < info.vertexAttributesInfo.size(); i++)
		pPipeline->m_vertexAttributesInfo[i] = info.vertexAttributesInfo[i];

	pPipeline->m_vertexInputCreateInfo = {};
	pPipeline->m_vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pPipeline->m_vertexInputCreateInfo.vertexBindingDescriptionCount = pPipeline->m_vertexBindingsInfo.size();
	pPipeline->m_vertexInputCreateInfo.pVertexBindingDescriptions = pPipeline->m_vertexBindingsInfo.data();
	pPipeline->m_vertexInputCreateInfo.vertexAttributeDescriptionCount = pPipeline->m_vertexAttributesInfo.size();
	pPipeline->m_vertexInputCreateInfo.pVertexAttributeDescriptions = pPipeline->m_vertexAttributesInfo.data();

	pPipeline->m_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pPipeline->m_info.pColorBlendState		= &pPipeline->m_blendCreateInfo;
	pPipeline->m_info.pDepthStencilState	= &pPipeline->m_depthStencilCreateInfo;
	pPipeline->m_info.pInputAssemblyState	= &pPipeline->m_assemblyCreateInfo;
	pPipeline->m_info.pMultisampleState		= &pPipeline->m_multiSampleCreateInfo;
	pPipeline->m_info.pRasterizationState	= &pPipeline->m_rasterizerCreateInfo;
	pPipeline->m_info.pViewportState		= &pPipeline->m_viewportStateCreateInfo;
	pPipeline->m_info.pDynamicState			= &pPipeline->m_dynamicStatesCreateInfo;
	pPipeline->m_info.renderPass			= info.pRenderPass->GetDeviceHandle();
	pPipeline->m_info.layout				= info.pPipelineLayout->GetDeviceHandle();
	pPipeline->m_info.pVertexInputState		= &pPipeline->m_vertexInputCreateInfo;

	if (pPipeline.get() && pPipeline->Init(pDevice))
		return pPipeline;
	return nullptr;
}