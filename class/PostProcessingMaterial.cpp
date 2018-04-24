#include "PostProcessingMaterial.h"
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
#include "GBufferPass.h"
#include "ShadowMapPass.h"
#include "FrameBufferDiction.h"

std::shared_ptr<PostProcessingMaterial> PostProcessingMaterial::CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo)
{
	std::shared_ptr<PostProcessingMaterial> pPostProcessMaterial = std::make_shared<PostProcessingMaterial>();

	VkGraphicsPipelineCreateInfo createInfo = {};

	std::vector<VkPipelineColorBlendAttachmentState> blendStatesInfo;
	uint32_t colorTargetCount = FrameBufferDiction::GetInstance()->GetFrameBuffer(simpleMaterialInfo.frameBufferType)->GetColorTargets().size();

	for (uint32_t i = 0; i < colorTargetCount; i++)
	{
		blendStatesInfo.push_back(
			{
				VK_FALSE,							// blend enabled

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
	depthStencilCreateInfo.depthTestEnable = VK_FALSE;
	depthStencilCreateInfo.depthWriteEnable = VK_FALSE;
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

	std::vector<VkVertexInputBindingDescription> vertexBindingsInfo(simpleMaterialInfo.vertexBindingsInfo.size());
	for (uint32_t i = 0; i < simpleMaterialInfo.vertexBindingsInfo.size(); i++)
		vertexBindingsInfo[i] = simpleMaterialInfo.vertexBindingsInfo[i];

	std::vector<VkVertexInputAttributeDescription> vertexAttributesInfo(simpleMaterialInfo.vertexAttributesInfo.size());
	for (uint32_t i = 0; i < simpleMaterialInfo.vertexAttributesInfo.size(); i++)
		vertexAttributesInfo[i] = simpleMaterialInfo.vertexAttributesInfo[i];

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
	createInfo.renderPass = simpleMaterialInfo.pRenderPass->GetRenderPass()->GetDeviceHandle();
	createInfo.subpass = simpleMaterialInfo.subpassIndex;

	if (pPostProcessMaterial.get() && pPostProcessMaterial->Init(pPostProcessMaterial, simpleMaterialInfo.shaderPaths, simpleMaterialInfo.pRenderPass, createInfo, simpleMaterialInfo.materialUniformVars, simpleMaterialInfo.vertexFormat))
		return pPostProcessMaterial;

	return nullptr;
}

bool PostProcessingMaterial::Init(const std::shared_ptr<PostProcessingMaterial>& pSelf,
	const std::vector<std::wstring>	shaderPaths,
	const std::shared_ptr<RenderPassBase>& pRenderPass,
	const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
	const std::vector<UniformVar>& materialUniformVars,
	uint32_t vertexFormat)
{
	if (!Material::Init(pSelf, shaderPaths, pRenderPass, pipelineCreateInfo, materialUniformVars, vertexFormat))
		return false;

	std::shared_ptr<RenderPassBase> pGBufferPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer);

	std::vector<CombinedImage> shadingResults;
	std::vector<CombinedImage> bloomTextures;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];

		shadingResults.push_back({
			pShadingFrameBuffer->GetColorTarget(0),
			pShadingFrameBuffer->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pShadingFrameBuffer->GetColorTarget(0)->CreateDefaultImageView()
		});

		std::shared_ptr<FrameBuffer> pBloomFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_BloomBlurH)[j];

		bloomTextures.push_back({
			pBloomFrameBuffer->GetColorTarget(0),
			pBloomFrameBuffer->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pBloomFrameBuffer->GetColorTarget(0)->CreateDefaultImageView()
		});
	}

	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount, shadingResults);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 1, bloomTextures);

	return true;
}

void PostProcessingMaterial::CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout)
{
		m_materialVariableLayout.push_back(
			{
				CombinedSampler,
				"Shading Result",
				{},
				GetSwapChain()->GetSwapChainImageCount()
			});

		m_materialVariableLayout.push_back(
			{
				CombinedSampler,
				"Bloom Texture",
				{},
				GetSwapChain()->GetSwapChainImageCount()
			});
}

void PostProcessingMaterial::CustomizePoolSize(std::vector<uint32_t>& counts)
{
	counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += (GetSwapChain()->GetSwapChainImageCount() * 2);
}

void PostProcessingMaterial::Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer)
{
	std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadPerFrameRes()->AllocateSecondaryCommandBuffer();

	pDrawCmdBuffer->StartSecondaryRecording(m_pRenderPass->GetRenderPass(), m_pPipeline->GetInfo().subpass, pFrameBuffer);

	VkViewport viewport =
	{
		0, 0,
		pFrameBuffer->GetFramebufferInfo().width, pFrameBuffer->GetFramebufferInfo().height,
		0, 1
	};

	VkRect2D scissorRect =
	{
		0, 0,
		pFrameBuffer->GetFramebufferInfo().width, pFrameBuffer->GetFramebufferInfo().height,
	};

	pDrawCmdBuffer->SetViewports({ GetGlobalVulkanStates()->GetViewport() });
	pDrawCmdBuffer->SetScissors({ GetGlobalVulkanStates()->GetScissorRect() });

	BindPipeline(pDrawCmdBuffer);
	BindDescriptorSet(pDrawCmdBuffer);
	BindMeshData(pDrawCmdBuffer);

	pDrawCmdBuffer->DrawIndexed(3);

	pDrawCmdBuffer->EndSecondaryRecording();

	pCmdBuf->Execute({ pDrawCmdBuffer });
}