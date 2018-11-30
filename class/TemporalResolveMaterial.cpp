#include "TemporalResolveMaterial.h"
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
#include "FrameBufferDiction.h"
#include "../common/Util.h"

std::shared_ptr<TemporalResolveMaterial> TemporalResolveMaterial::CreateDefaultMaterial(uint32_t pingpong)
{
	SimpleMaterialCreateInfo simpleMaterialInfo = {};
	simpleMaterialInfo.shaderPaths = { L"../data/shaders/screen_quad_vert_recon.vert.spv", L"", L"", L"", L"../data/shaders/temporal_resolve.frag.spv", L"" };
	simpleMaterialInfo.vertexFormat = VertexFormatNul;
	simpleMaterialInfo.subpassIndex = 0;
	simpleMaterialInfo.frameBufferType = FrameBufferDiction::FrameBufferType_TemporalResolve;
	simpleMaterialInfo.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassTemporalResolve);
	simpleMaterialInfo.depthTestEnable = false;
	simpleMaterialInfo.depthWriteEnable = false;

	std::shared_ptr<TemporalResolveMaterial> pResolveMaterial = std::make_shared<TemporalResolveMaterial>();

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

	pResolveMaterial->m_pingPong = (pingpong + 1) % 2;

	if (pResolveMaterial.get() && pResolveMaterial->Init(pResolveMaterial, simpleMaterialInfo.shaderPaths, simpleMaterialInfo.pRenderPass, createInfo, simpleMaterialInfo.materialUniformVars, simpleMaterialInfo.vertexFormat))
		return pResolveMaterial;

	return nullptr;
}

bool TemporalResolveMaterial::Init(const std::shared_ptr<TemporalResolveMaterial>& pSelf,
	const std::vector<std::wstring>	shaderPaths,
	const std::shared_ptr<RenderPassBase>& pRenderPass,
	const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
	const std::vector<UniformVar>& materialUniformVars,
	uint32_t vertexFormat)
{
	if (!Material::Init(pSelf, shaderPaths, pRenderPass, pipelineCreateInfo, materialUniformVars, vertexFormat))
		return false;

	for (uint32_t i = 0; i < FrameBufferDiction::GBufferCount + 1; i++)
	{
		std::vector<CombinedImage> gbuffers;
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pGBufferFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];

			if (i < FrameBufferDiction::GBufferCount)
				gbuffers.push_back({
					pGBufferFrameBuffer->GetColorTarget(i),
					pGBufferFrameBuffer->GetColorTarget(i)->CreateLinearClampToEdgeSampler(),
					pGBufferFrameBuffer->GetColorTarget(i)->CreateDefaultImageView()
				});
			else
				gbuffers.push_back({
					pGBufferFrameBuffer->GetDepthStencilTarget(),
					pGBufferFrameBuffer->GetDepthStencilTarget()->CreateLinearClampToEdgeSampler(),
					pGBufferFrameBuffer->GetDepthStencilTarget()->CreateDepthSampleImageView()
				});
		}

		m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + i, gbuffers);
	}

	std::vector<CombinedImage> shadingResults;
	std::vector<CombinedImage> EnvReflResults;
	std::vector<CombinedImage> motionVectors;
	std::vector<CombinedImage> SSRInfo;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];

		shadingResults.push_back({
			pShadingResult->GetColorTarget(0),
			pShadingResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pShadingResult->GetColorTarget(0)->CreateDefaultImageView()
			});

		std::shared_ptr<FrameBuffer> pEnvReflResults = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];

		EnvReflResults.push_back({
			pEnvReflResults->GetColorTarget(1),
			pEnvReflResults->GetColorTarget(1)->CreateLinearClampToEdgeSampler(),
			pEnvReflResults->GetColorTarget(1)->CreateDefaultImageView()
			});

		std::shared_ptr<FrameBuffer> pSSRInfo = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_SSAOSSR)[j];

		SSRInfo.push_back({
			pSSRInfo->GetColorTarget(1),
			pSSRInfo->GetColorTarget(1)->CreateLinearClampToEdgeSampler(),
			pSSRInfo->GetColorTarget(1)->CreateDefaultImageView()
		});
	}

	std::vector<CombinedImage> temporalShading;
	temporalShading.push_back({
		FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, m_pingPong)->GetColorTarget(FrameBufferDiction::TemporalShading),
		FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, m_pingPong)->GetColorTarget(FrameBufferDiction::TemporalShading)->CreateLinearClampToEdgeSampler(),
		FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, m_pingPong)->GetColorTarget(FrameBufferDiction::TemporalShading)->CreateDefaultImageView()
		});

	std::vector<CombinedImage> temporalSSR;
	temporalSSR.push_back({
		FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, m_pingPong)->GetColorTarget(FrameBufferDiction::TemporalSSR),
		FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, m_pingPong)->GetColorTarget(FrameBufferDiction::TemporalSSR)->CreateLinearClampToEdgeSampler(),
		FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, m_pingPong)->GetColorTarget(FrameBufferDiction::TemporalSSR)->CreateDefaultImageView()
		});

	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + FrameBufferDiction::GBufferCount + 1, shadingResults);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + FrameBufferDiction::GBufferCount + 2, EnvReflResults);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + FrameBufferDiction::GBufferCount + 3, SSRInfo);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + FrameBufferDiction::GBufferCount + 4, temporalShading);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + FrameBufferDiction::GBufferCount + 5, temporalSSR);

	return true;
}

void TemporalResolveMaterial::CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout)
{
	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"GBuffer0",
		{},
		GetSwapChain()->GetSwapChainImageCount()
	});

	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"GBuffer1",
		{},
		GetSwapChain()->GetSwapChainImageCount()
	});

	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"GBuffer2",
		{},
		GetSwapChain()->GetSwapChainImageCount()
	});

	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"MotionVector",
		{},
		GetSwapChain()->GetSwapChainImageCount()
	});

	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"DepthBuffer",
		{},
		GetSwapChain()->GetSwapChainImageCount()
	});

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
		"SSR Result",
		{},
		GetSwapChain()->GetSwapChainImageCount()
	});

	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"SSR Info",
		{},
		GetSwapChain()->GetSwapChainImageCount()
	});

	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"Temporal Shading",
		{},
	});

	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"Temporal SSR",
		{},
	});
}

void TemporalResolveMaterial::CustomizePoolSize(std::vector<uint32_t>& counts)
{
	counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += (GetSwapChain()->GetSwapChainImageCount() * 2);
}

void TemporalResolveMaterial::Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong)
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

void TemporalResolveMaterial::AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, uint32_t pingpong)
{
	std::vector<VkImageMemoryBarrier> barriers;
	for (uint32_t i = 0; i < FrameBufferDiction::GBufferCount; i++)
	{
		std::shared_ptr<Image> pGBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[FrameMgr()->FrameIndex()]->GetColorTarget(i);

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = pGBuffer->GetImageInfo().mipLevels;
		subresourceRange.layerCount = pGBuffer->GetImageInfo().arrayLayers;

		VkImageMemoryBarrier imgBarrier = {};
		imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imgBarrier.image = pGBuffer->GetDeviceHandle();
		imgBarrier.subresourceRange = subresourceRange;
		imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		barriers.push_back(imgBarrier);
	}

	std::shared_ptr<Image> pShadingResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[FrameMgr()->FrameIndex()]->GetColorTarget(0);
	std::shared_ptr<Image> pSSRResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[FrameMgr()->FrameIndex()]->GetColorTarget(1);
	std::shared_ptr<Image> pMotionVector = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[FrameMgr()->FrameIndex()]->GetColorTarget(FrameBufferDiction::MotionVector);
	std::shared_ptr<Image> pSSRInfo = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_SSAOSSR)[FrameMgr()->FrameIndex()]->GetColorTarget(1);
	std::shared_ptr<Image> pTemporalShading = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, m_pingPong)->GetColorTarget(FrameBufferDiction::TemporalShading);
	std::shared_ptr<Image> pTemporalSSR = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, m_pingPong)->GetColorTarget(FrameBufferDiction::TemporalSSR);

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pShadingResult->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pShadingResult->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier = {};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pShadingResult->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pSSRResult->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pSSRResult->GetImageInfo().arrayLayers;

	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pSSRResult->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pMotionVector->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pMotionVector->GetImageInfo().arrayLayers;

	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pMotionVector->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pSSRInfo->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pSSRInfo->GetImageInfo().arrayLayers;

	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pSSRInfo->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pTemporalShading->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pTemporalShading->GetImageInfo().arrayLayers;

	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pTemporalShading->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pTemporalSSR->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pTemporalSSR->GetImageInfo().arrayLayers;

	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pTemporalSSR->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		{},
		{},
		barriers
	);

	barriers.clear();

	std::shared_ptr<Image> pDepthTarget = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[FrameMgr()->FrameIndex()]->GetDepthStencilTarget();

	subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pDepthTarget->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pDepthTarget->GetImageInfo().arrayLayers;

	imgBarrier = {};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pDepthTarget->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		{},
		{},
		barriers
	);
}