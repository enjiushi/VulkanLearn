#include "GBufferPlanetMaterial.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/PerFrameResource.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/GlobalVulkanStates.h"
#include "../vulkan/SharedIndirectBuffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/DepthStencilBuffer.h"
#include "../vulkan/Texture2D.h"
#include "../vulkan/GraphicPipeline.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/DescriptorPool.h"
#include "../vulkan/DescriptorSetLayout.h"
#include "RenderPassBase.h"
#include "RenderPassDiction.h"
#include "RenderWorkManager.h"
#include "FrameBufferDiction.h"
#include "../common/Util.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "../class/PlanetGeoDataManager.h"

std::shared_ptr<GBufferPlanetMaterial> GBufferPlanetMaterial::CreateDefaultMaterial()
{
	std::vector<UniformVar> vars =
	{
		{
			{
				Vec4Unit,
				"AlbedoRoughness"
			},
			{
				Vec2Unit,
				"AOMetalic"
			},
			{
				OneUnit,
				"AlbedoRoughnessTextureIndex"
			},
			{
				OneUnit,
				"NormalAOTextureIndex"
			},
			{
				OneUnit,
				"MetallicTextureIndex"
			}
		}
	};

	SimpleMaterialCreateInfo simpleMaterialInfo = {};
	simpleMaterialInfo.shaderPaths = { L"../data/shaders/pbr_gbuffer_planet.vert.spv", L"", L"", L"", L"../data/shaders/pbr_gbuffer_planet.frag.spv", L"" };
	simpleMaterialInfo.materialUniformVars = vars;
	simpleMaterialInfo.vertexFormat =  VertexFormatP;
	simpleMaterialInfo.vertexFormatInMem = VertexFormatP;
	simpleMaterialInfo.subpassIndex = 0;
	simpleMaterialInfo.frameBufferType = FrameBufferDiction::FrameBufferType_GBuffer;
	simpleMaterialInfo.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer);

	std::shared_ptr<GBufferPlanetMaterial> pGBufferPlanetMaterial = std::make_shared<GBufferPlanetMaterial>();

	VkGraphicsPipelineCreateInfo createInfo = {};

	std::vector<VkPipelineColorBlendAttachmentState> blendStatesInfo;
	uint32_t colorTargetCount = FrameBufferDiction::GetInstance()->GetFrameBuffer(simpleMaterialInfo.frameBufferType)->GetColorTargets().size();

	for (uint32_t i = 0; i < colorTargetCount; i++)
	{
		blendStatesInfo.push_back(
			{
				simpleMaterialInfo.isTransparent,	// blend enabled

				VK_BLEND_FACTOR_SRC_ALPHA,			// src color blend factor
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,// dst color blend factor
				VK_BLEND_OP_ADD,					// color blend op

				VK_BLEND_FACTOR_ONE,				// src alpha blend factor
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,// dst alpha blend factor
				VK_BLEND_OP_ADD,					// alpha blend factor

				0xf,								// color mask
			}
		);
	}

	VkPipelineColorBlendStateCreateInfo blendCreateInfo = {};
	blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendCreateInfo.logicOpEnable = VK_FALSE;
	blendCreateInfo.attachmentCount = blendStatesInfo.size();
	blendCreateInfo.pAttachments = blendStatesInfo.data();

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = simpleMaterialInfo.depthTestEnable;
	depthStencilCreateInfo.depthWriteEnable = simpleMaterialInfo.depthWriteEnable;
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;

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

	std::vector<VkVertexInputBindingDescription> vertexBindingsInfo;
	std::vector<VkVertexInputAttributeDescription> vertexAttributesInfo;
	if (simpleMaterialInfo.vertexFormat)
	{
		vertexBindingsInfo.push_back(GenerateBindingDesc(0, simpleMaterialInfo.vertexFormatInMem));
		vertexAttributesInfo = GenerateAttribDesc(0, simpleMaterialInfo.vertexFormat, simpleMaterialInfo.vertexFormatInMem);
	}

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
	createInfo.pVertexInputState = &vertexInputCreateInfo;
	createInfo.subpass = simpleMaterialInfo.subpassIndex;
	createInfo.renderPass = simpleMaterialInfo.pRenderPass->GetRenderPass()->GetDeviceHandle();

	if (pGBufferPlanetMaterial.get() && pGBufferPlanetMaterial->Init(pGBufferPlanetMaterial, simpleMaterialInfo.shaderPaths, simpleMaterialInfo.pRenderPass, createInfo, simpleMaterialInfo.materialUniformVars, simpleMaterialInfo.vertexFormat, simpleMaterialInfo.vertexFormatInMem, true))
		return pGBufferPlanetMaterial;
	return nullptr;
}

void GBufferPlanetMaterial::CustomizeSecondaryCmd(const std::shared_ptr<CommandBuffer>& pSecondaryCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong)
{
	std::shared_ptr<PerFrameBuffer> pPerFrameBuffer = PlanetGeoDataManager::GetInstance()->GetPerFrameBuffer();
	pSecondaryCmdBuf->BindVertexBuffer(pPerFrameBuffer->GetBuffer(), FrameMgr()->FrameIndex() * pPerFrameBuffer->GetFrameOffset(), 1);
}