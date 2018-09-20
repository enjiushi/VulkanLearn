#include "TemporalMaterial.h"
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

std::shared_ptr<TemporalMaterial> TemporalMaterial::CreateDefaultMaterial(uint32_t pingpong)
{
	SimpleMaterialCreateInfo simpleMaterialInfo = {};
	simpleMaterialInfo.shaderPaths = { L"../data/shaders/screen_quad.vert.spv", L"", L"", L"", L"../data/shaders/temporal.frag.spv", L"" };
	simpleMaterialInfo.vertexFormat = VertexFormatNul;
	simpleMaterialInfo.subpassIndex = 0;
	simpleMaterialInfo.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassTemporal);
	simpleMaterialInfo.frameBufferType = FrameBufferDiction::FrameBufferType_Temporal0;	// 0 and 1 are the same
	simpleMaterialInfo.depthTestEnable = false;
	simpleMaterialInfo.depthWriteEnable = false;

	std::shared_ptr<TemporalMaterial> pTemporalMaterial = std::make_shared<TemporalMaterial>();

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

	VkPushConstantRange pushConstantRange0 = { VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t) };

	pTemporalMaterial->m_pingPong = (pingpong + 1) % 2;

	if (pTemporalMaterial.get() && pTemporalMaterial->Init(pTemporalMaterial, simpleMaterialInfo.shaderPaths, simpleMaterialInfo.pRenderPass, createInfo, simpleMaterialInfo.materialUniformVars, { pushConstantRange0 }, simpleMaterialInfo.vertexFormat))
		return pTemporalMaterial;

	return nullptr;
}

bool TemporalMaterial::Init(const std::shared_ptr<TemporalMaterial>& pSelf,
	const std::vector<std::wstring>	shaderPaths,
	const std::shared_ptr<RenderPassBase>& pRenderPass,
	const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
	const std::vector<UniformVar>& materialUniformVars,
	const std::vector<VkPushConstantRange>& pushConstsRanges,
	uint32_t vertexFormat)
{
	if (!Material::Init(pSelf, shaderPaths, pRenderPass, pipelineCreateInfo, pushConstsRanges, materialUniformVars, vertexFormat))
		return false;

	std::vector<CombinedImage> shadingResults;
	std::vector<CombinedImage> motionVectors;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];

		shadingResults.push_back({
			pShadingResult->GetColorTarget(0),
			pShadingResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pShadingResult->GetColorTarget(0)->CreateDefaultImageView()
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
		FrameBufferDiction::GetInstance()->GetFrameBuffers((FrameBufferDiction::FrameBufferType)(FrameBufferDiction::FrameBufferType_Temporal0 + m_pingPong))[0]->GetColorTarget(0),
		FrameBufferDiction::GetInstance()->GetFrameBuffers((FrameBufferDiction::FrameBufferType)(FrameBufferDiction::FrameBufferType_Temporal0 + m_pingPong))[0]->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
		FrameBufferDiction::GetInstance()->GetFrameBuffers((FrameBufferDiction::FrameBufferType)(FrameBufferDiction::FrameBufferType_Temporal0 + m_pingPong))[0]->GetColorTarget(0)->CreateDefaultImageView()
		});

	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount, shadingResults);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 1, motionVectors);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 2, temporalBuffers);

	return true;
}

void TemporalMaterial::CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout)
{
	materialLayout.push_back(
	{
		CombinedSampler,
		"Shading Result",
		{},
		GetSwapChain()->GetSwapChainImageCount()
	});

	materialLayout.push_back(
	{
		CombinedSampler,
		"Motion Vector",
		{},
		GetSwapChain()->GetSwapChainImageCount()
	});

	materialLayout.push_back(
	{
		CombinedSampler,
		"Temporal History",
		{}
	});
}

void TemporalMaterial::CustomizePoolSize(std::vector<uint32_t>& counts)
{
	counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += (GetSwapChain()->GetSwapChainImageCount() + 2);
}

void TemporalMaterial::Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer)
{
	std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadPerFrameRes()->AllocateSecondaryCommandBuffer();

	// FIXME: Hard-coded subpass index, which should be defined somewhere as an enum
	pDrawCmdBuffer->StartSecondaryRecording(m_pRenderPass->GetRenderPass(), m_pPipeline->GetInfo().subpass, pFrameBuffer);

	pDrawCmdBuffer->SetViewports({ GetGlobalVulkanStates()->GetViewport() });
	pDrawCmdBuffer->SetScissors({ GetGlobalVulkanStates()->GetScissorRect() });

	BindPipeline(pDrawCmdBuffer);
	BindDescriptorSet(pDrawCmdBuffer);

	pDrawCmdBuffer->PushConstants(m_pPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &m_pingPong);

	pDrawCmdBuffer->Draw(3, 1, 0, 0);

	pDrawCmdBuffer->EndSecondaryRecording();

	pCmdBuf->Execute({ pDrawCmdBuffer });
}

void TemporalMaterial::AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	std::vector<VkImageMemoryBarrier> barriers;

	std::shared_ptr<Image> pShadingResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[FrameMgr()->FrameIndex()]->GetColorTarget(0);
	std::shared_ptr<Image> pTemporalHistory = FrameBufferDiction::GetInstance()->GetFrameBuffers((FrameBufferDiction::FrameBufferType)(FrameBufferDiction::FrameBufferType_Temporal0 + m_pingPong))[0]->GetColorTarget(0);
	std::shared_ptr<Image> pMotionVector = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[FrameMgr()->FrameIndex()]->GetColorTarget(FrameBufferDiction::MotionVector);

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
	subresourceRange1.levelCount = pMotionVector->GetImageInfo().mipLevels;
	subresourceRange1.layerCount = pMotionVector->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier1 = {};
	imgBarrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier1.image = pMotionVector->GetDeviceHandle();
	imgBarrier1.subresourceRange = subresourceRange1;
	imgBarrier1.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier1.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier1.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier1.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkImageSubresourceRange subresourceRange2 = {};
	subresourceRange2.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange2.baseMipLevel = 0;
	subresourceRange2.levelCount = pTemporalHistory->GetImageInfo().mipLevels;
	subresourceRange2.layerCount = pTemporalHistory->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier2 = {};
	imgBarrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier2.image = pTemporalHistory->GetDeviceHandle();
	imgBarrier2.subresourceRange = subresourceRange0;
	imgBarrier2.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		{},
		{},
		{ imgBarrier0, imgBarrier1, imgBarrier2 }
	);
}