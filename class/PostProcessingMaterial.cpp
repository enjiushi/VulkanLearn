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
#include "../common/Util.h"

std::shared_ptr<PostProcessingMaterial> PostProcessingMaterial::CreateDefaultMaterial(uint32_t pingpong)
{
	SimpleMaterialCreateInfo simpleMaterialInfo = {};
	simpleMaterialInfo.shaderPaths = { L"../data/shaders/screen_quad.vert.spv", L"", L"", L"", L"../data/shaders/post_processing.frag.spv", L"" };
	simpleMaterialInfo.vertexFormat = VertexFormatNul;
	simpleMaterialInfo.subpassIndex = 0;
	simpleMaterialInfo.frameBufferType = FrameBufferDiction::FrameBufferType_PostProcessing;
	simpleMaterialInfo.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassPostProcessing);
	simpleMaterialInfo.depthTestEnable = false;
	simpleMaterialInfo.depthWriteEnable = false;

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

	std::vector<VkVertexInputBindingDescription> vertexBindingsInfo;
	std::vector<VkVertexInputAttributeDescription> vertexAttributesInfo;
	if (simpleMaterialInfo.vertexFormat)
	{
		vertexBindingsInfo.push_back(GenerateBindingDesc(0, simpleMaterialInfo.vertexFormat));
		vertexAttributesInfo = GenerateAttribDesc(0, simpleMaterialInfo.vertexFormat);
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
	createInfo.renderPass = simpleMaterialInfo.pRenderPass->GetRenderPass()->GetDeviceHandle();
	createInfo.subpass = simpleMaterialInfo.subpassIndex;

	pPostProcessMaterial->m_pingPong = (pingpong + 1) % 2;

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
	std::vector<CombinedImage> motionVectors;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];

		shadingResults.push_back({
			pShadingResult->GetColorTarget(0),
			pShadingResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pShadingResult->GetColorTarget(0)->CreateDefaultImageView()
			});

		std::shared_ptr<FrameBuffer> pBloomFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_BloomBlurH)[j];

		bloomTextures.push_back({
			pBloomFrameBuffer->GetColorTarget(0),
			pBloomFrameBuffer->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pBloomFrameBuffer->GetColorTarget(0)->CreateDefaultImageView()
		});

		std::shared_ptr<FrameBuffer> pMotionVector = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];

		motionVectors.push_back({
			pMotionVector->GetColorTarget(FrameBufferDiction::MotionVector),
			pMotionVector->GetColorTarget(FrameBufferDiction::MotionVector)->CreateLinearClampToEdgeSampler(),
			pMotionVector->GetColorTarget(FrameBufferDiction::MotionVector)->CreateDefaultImageView()
		});
	}

	std::vector<CombinedImage> temporalBuffers;
	temporalBuffers.push_back({
		FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_PostProcessing, m_pingPong)->GetColorTarget(1),
		FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_PostProcessing, m_pingPong)->GetColorTarget(1)->CreateLinearClampToEdgeSampler(),
		FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_PostProcessing, m_pingPong)->GetColorTarget(1)->CreateDefaultImageView()
		});

	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount, shadingResults);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 1, bloomTextures);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 2, motionVectors);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 3, temporalBuffers);

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

		m_materialVariableLayout.push_back(
			{
				CombinedSampler,
				"Motion Vector",
				{},
				GetSwapChain()->GetSwapChainImageCount()
			});

		m_materialVariableLayout.push_back(
			{
				CombinedSampler,
				"Temporal History",
				{},
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

	pDrawCmdBuffer->SetViewports({ GetGlobalVulkanStates()->GetViewport() });
	pDrawCmdBuffer->SetScissors({ GetGlobalVulkanStates()->GetScissorRect() });

	BindPipeline(pDrawCmdBuffer);
	BindDescriptorSet(pDrawCmdBuffer);

	pDrawCmdBuffer->Draw(3, 1, 0, 0);

	pDrawCmdBuffer->EndSecondaryRecording();

	pCmdBuf->Execute({ pDrawCmdBuffer });
}

void PostProcessingMaterial::AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	std::shared_ptr<Image> pShadingResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[FrameMgr()->FrameIndex()]->GetColorTarget(0);
	std::shared_ptr<Image> pBloomTex = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_BloomBlurH)[FrameMgr()->FrameIndex()]->GetColorTarget(0);
	std::shared_ptr<Image> pMotionVector = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[FrameMgr()->FrameIndex()]->GetColorTarget(FrameBufferDiction::MotionVector);
	std::shared_ptr<Image> pTemporalHistory = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_PostProcessing, m_pingPong)->GetColorTarget(1);

	VkImageSubresourceRange subresourceRange0 = {};
	subresourceRange0.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange0.baseMipLevel = 0;
	subresourceRange0.levelCount = pShadingResult->GetImageInfo().mipLevels;
	subresourceRange0.layerCount = pShadingResult->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier0 = {};
	imgBarrier0.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier0.image = pShadingResult->GetDeviceHandle();
	imgBarrier0.subresourceRange = subresourceRange0;
	imgBarrier0.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier0.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier0.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier0.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkImageSubresourceRange subresourceRange1 = {};
	subresourceRange1.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange1.baseMipLevel = 0;
	subresourceRange1.levelCount = pBloomTex->GetImageInfo().mipLevels;
	subresourceRange1.layerCount = pBloomTex->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier1 = {};
	imgBarrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier1.image = pBloomTex->GetDeviceHandle();
	imgBarrier1.subresourceRange = subresourceRange1;
	imgBarrier1.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier1.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier1.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier1.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkImageSubresourceRange subresourceRange2 = {};
	subresourceRange2.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange2.baseMipLevel = 0;
	subresourceRange2.levelCount = pMotionVector->GetImageInfo().mipLevels;
	subresourceRange2.layerCount = pMotionVector->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier2 = {};
	imgBarrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier2.image = pMotionVector->GetDeviceHandle();
	imgBarrier2.subresourceRange = subresourceRange2;
	imgBarrier2.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkImageSubresourceRange subresourceRange3 = {};
	subresourceRange3.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange3.baseMipLevel = 0;
	subresourceRange3.levelCount = pTemporalHistory->GetImageInfo().mipLevels;
	subresourceRange3.layerCount = pTemporalHistory->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier3 = {};
	imgBarrier3.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier3.image = pTemporalHistory->GetDeviceHandle();
	imgBarrier3.subresourceRange = subresourceRange3;
	imgBarrier3.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier3.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier3.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier3.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		{},
		{},
		{ imgBarrier0, imgBarrier1, imgBarrier2, imgBarrier3 }
	);
}