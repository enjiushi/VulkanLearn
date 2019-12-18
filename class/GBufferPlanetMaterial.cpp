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
	uint32_t colorTargetCount = (uint32_t)FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer)->GetColorTargets().size();

	for (uint32_t i = 0; i < colorTargetCount; i++)
	{
		blendStatesInfo.push_back(
			{
				false,								// blend enabled

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
	blendCreateInfo.attachmentCount = (uint32_t)blendStatesInfo.size();
	blendCreateInfo.pAttachments = blendStatesInfo.data();

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = true;
	depthStencilCreateInfo.depthWriteEnable = true;
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
	dynamicStatesCreateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicStatesCreateInfo.pDynamicStates = dynamicStates.data();

	std::vector<VkVertexInputBindingDescription> vertexBindingsInfo;
	std::vector<VkVertexInputAttributeDescription> vertexAttributesInfo;
	if (simpleMaterialInfo.vertexFormat)
	{
		// FIXME: this place needs a serious refactor
		vertexBindingsInfo.push_back(GenerateReservedVBBindingDesc(simpleMaterialInfo.vertexFormatInMem));
		vertexAttributesInfo = GenerateReservedVBAttribDesc(simpleMaterialInfo.vertexFormat, simpleMaterialInfo.vertexFormatInMem);

		VkVertexInputBindingDescription bindingDesc = {};
		bindingDesc.binding = 1;
		bindingDesc.stride = 3 * sizeof(Vector3f);
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
		vertexBindingsInfo.push_back(bindingDesc);

		VkVertexInputAttributeDescription attribDesc = {};
		attribDesc.binding = 1;
		attribDesc.location = 1;
		attribDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDesc.offset = 0;
		vertexAttributesInfo.push_back(attribDesc);

		attribDesc = {};
		attribDesc.binding = 1;
		attribDesc.location = 2;
		attribDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDesc.offset = sizeof(Vector3f);
		vertexAttributesInfo.push_back(attribDesc);

		attribDesc = {};
		attribDesc.binding = 1;
		attribDesc.location = 3;
		attribDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDesc.offset = 2 * sizeof(Vector3f);
		vertexAttributesInfo.push_back(attribDesc);
	}

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = (uint32_t)vertexBindingsInfo.size();
	vertexInputCreateInfo.pVertexBindingDescriptions = vertexBindingsInfo.data();
	vertexInputCreateInfo.vertexAttributeDescriptionCount = (uint32_t)vertexAttributesInfo.size();
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
	createInfo.subpass = 0;	// FIXME
	createInfo.renderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->GetRenderPass()->GetDeviceHandle();

	if (pGBufferPlanetMaterial.get() && pGBufferPlanetMaterial->Init(
		pGBufferPlanetMaterial, 
		{ L"../data/shaders/pbr_gbuffer_planet.vert.spv", L"", L"", L"", L"../data/shaders/pbr_gbuffer_planet.frag.spv", L"" }, 
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer),
		createInfo, 
		simpleMaterialInfo.materialUniformVars, 
		simpleMaterialInfo.vertexFormat,
		simpleMaterialInfo.vertexFormatInMem, 
		true))
		return pGBufferPlanetMaterial;
	return nullptr;
}

void GBufferPlanetMaterial::CustomizeSecondaryCmd(const std::shared_ptr<CommandBuffer>& pSecondaryCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong)
{
	std::shared_ptr<PerFrameBuffer> pPerFrameBuffer = PlanetGeoDataManager::GetInstance()->GetPerFrameBuffer();
	pSecondaryCmdBuf->BindVertexBuffer(pPerFrameBuffer->GetBuffer(), FrameMgr()->FrameIndex() * pPerFrameBuffer->GetFrameOffset(), 1);
}