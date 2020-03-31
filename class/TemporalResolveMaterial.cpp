#include "TemporalResolveMaterial.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/PerFrameResource.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/GlobalVulkanStates.h"
#include "../vulkan/SharedIndirectBuffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/Image.h"
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
	simpleMaterialInfo.shaderPaths = { L"../data/shaders/screen_quad.vert.spv", L"", L"", L"", L"../data/shaders/temporal_resolve.frag.spv", L"" };
	simpleMaterialInfo.vertexFormat = VertexFormatNul;
	simpleMaterialInfo.vertexFormatInMem = VertexFormatNul;
	simpleMaterialInfo.subpassIndex = 0;
	simpleMaterialInfo.frameBufferType = FrameBufferDiction::FrameBufferType_TemporalResolve;
	simpleMaterialInfo.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassTemporalResolve);
	simpleMaterialInfo.depthTestEnable = false;
	simpleMaterialInfo.depthWriteEnable = false;

	std::shared_ptr<TemporalResolveMaterial> pResolveMaterial = std::make_shared<TemporalResolveMaterial>();

	VkGraphicsPipelineCreateInfo createInfo = {};

	std::vector<VkPipelineColorBlendAttachmentState> blendStatesInfo;
	uint32_t colorTargetCount = (uint32_t)FrameBufferDiction::GetInstance()->GetFrameBuffer(simpleMaterialInfo.frameBufferType)->GetColorTargets().size();

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
	blendCreateInfo.attachmentCount = (uint32_t)blendStatesInfo.size();
	blendCreateInfo.pAttachments = blendStatesInfo.data();

	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_FALSE;
	depthStencilCreateInfo.depthWriteEnable = VK_FALSE;
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

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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

	if (pResolveMaterial.get() && pResolveMaterial->Init(pResolveMaterial, simpleMaterialInfo.shaderPaths, simpleMaterialInfo.pRenderPass, createInfo, simpleMaterialInfo.materialUniformVars, simpleMaterialInfo.vertexFormat, simpleMaterialInfo.vertexFormatInMem, pingpong))
		return pResolveMaterial;

	return nullptr;
}

bool TemporalResolveMaterial::Init(const std::shared_ptr<TemporalResolveMaterial>& pSelf,
	const std::vector<std::wstring>	shaderPaths,
	const std::shared_ptr<RenderPassBase>& pRenderPass,
	const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
	const std::vector<UniformVar>& materialUniformVars,
	uint32_t vertexFormat,
	uint32_t vertexFormatInMem,
	uint32_t pingpong)
{
	if (!Material::Init(pSelf, shaderPaths, pRenderPass, pipelineCreateInfo, materialUniformVars, vertexFormat, vertexFormatInMem, false))
		return false;

	std::vector<CombinedImage> motionVectors;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pGBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];

		motionVectors.push_back({
			pGBuffer->GetColorTarget(FrameBufferDiction::MotionVector),
			pGBuffer->GetColorTarget(FrameBufferDiction::MotionVector)->CreateLinearClampToEdgeSampler(),
			pGBuffer->GetColorTarget(FrameBufferDiction::MotionVector)->CreateDefaultImageView()
		});
	}

	std::vector<CombinedImage> shadingResults;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingResultBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];

		shadingResults.push_back({
			pShadingResultBuffer->GetColorTarget(0),
			pShadingResultBuffer->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pShadingResultBuffer->GetColorTarget(0)->CreateDefaultImageView()
			});
	}

	std::vector<CombinedImage> SSRResults;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pShadingResultBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[j];

		SSRResults.push_back({
			pShadingResultBuffer->GetColorTarget(1),
			pShadingResultBuffer->GetColorTarget(1)->CreateLinearClampToEdgeSampler(),
			pShadingResultBuffer->GetColorTarget(1)->CreateDefaultImageView()
			});
	}


	std::vector<CombinedImage> GBuffer1;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pGBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];

		GBuffer1.push_back({
			pGBuffer->GetColorTarget(FrameBufferDiction::GBuffer1),
			pGBuffer->GetColorTarget(FrameBufferDiction::GBuffer1)->CreateLinearClampToEdgeSampler(),
			pGBuffer->GetColorTarget(FrameBufferDiction::GBuffer1)->CreateDefaultImageView()
			});
	}

	std::vector<CombinedImage> motionNeighborMax;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pMotionNeighborMax = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_MotionNeighborMax)[j];

		motionNeighborMax.push_back({
			pMotionNeighborMax->GetColorTarget(0),
			pMotionNeighborMax->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
			pMotionNeighborMax->GetColorTarget(0)->CreateDefaultImageView()
			});
	}

	std::vector<CombinedImage> temporalShadingResults;
	temporalShadingResults.push_back({
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::ShadingResult),
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::ShadingResult)->CreateLinearClampToEdgeSampler(),
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::ShadingResult)->CreateDefaultImageView()
		});

	std::vector<CombinedImage> temporalSSRResults;
	temporalSSRResults.push_back({
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::SSRResult),
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::SSRResult)->CreateLinearClampToEdgeSampler(),
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::SSRResult)->CreateDefaultImageView()
		});

	std::vector<CombinedImage> temporalResults;
	temporalResults.push_back({
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::CombinedResult),
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::CombinedResult)->CreateLinearClampToEdgeSampler(),
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::CombinedResult)->CreateDefaultImageView()
		});

	std::vector<CombinedImage> temporalCoC;
	temporalCoC.push_back({
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::CoC),
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::CoC)->CreateLinearClampToEdgeSampler(),
		FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::CoC)->CreateDefaultImageView()
		});

	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount, motionVectors);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 1, shadingResults);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 2, SSRResults);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 3, GBuffer1);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 4, motionNeighborMax);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 5, temporalShadingResults);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 6, temporalSSRResults);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 7, temporalResults);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 8, temporalCoC);

	return true;
}

void TemporalResolveMaterial::CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout)
{
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
		"ShadingResult",
		{},
		GetSwapChain()->GetSwapChainImageCount()
	});

	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"SSRResult",
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

	materialLayout.push_back(
	{
		CombinedSampler,
		"MotionNeighborMax",
		{},
		GetSwapChain()->GetSwapChainImageCount()
	});


	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"Temporal Shading Result",
		{},
		1
	});

	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"Temporal SSR Result",
		{},
		1
	});

	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"Temporal Result",
		{},
		1
	});

	m_materialVariableLayout.push_back(
	{
		CombinedSampler,
		"Temporal CoC",
		{},
		1
	});
}

void TemporalResolveMaterial::CustomizePoolSize(std::vector<uint32_t>& counts)
{
	counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += (GetSwapChain()->GetSwapChainImageCount() * 4 + 2);
}

void TemporalResolveMaterial::AfterRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, uint32_t pingpong)
{
	Material::AfterRenderPass(pCmdBuf, pingpong);

	std::shared_ptr<Image> pTemporalResult = FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, (pingpong + 1) % 2)->GetColorTarget(FrameBufferDiction::CombinedResult);
	std::shared_ptr<Image> pTextureArray = UniformData::GetInstance()->GetGlobalTextures()->GetScreenSizeTextureArray();

	uint32_t index;
	bool ret = UniformData::GetInstance()->GetGlobalTextures()->GetScreenSizeTextureIndex("MipmapTemporalResult", index);
	ASSERTION(ret && pTextureArray != nullptr);

	Vector2d windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();
	VkImageBlit blit =
	{
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			index,
			1
		},
		{ { 0, 0, 0 },{ (int32_t)windowSize.x, (int32_t)windowSize.y, 1 } },

		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			index,
			1
		},
		{ { 0, 0, 0 },{ (int32_t)windowSize.x, (int32_t)windowSize.y, 1 } }
	};
	pCmdBuf->BlitImage(pTemporalResult, pTextureArray, blit);
	pCmdBuf->GenerateMipmaps(pTextureArray, index);
}

void TemporalResolveMaterial::AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, BarrierInsertionPoint barrierInsertionPoint, uint32_t pingpong)
{
	if (barrierInsertionPoint == Material::BarrierInsertionPoint::AFTER_DISPATCH)
		return;

	std::vector<VkImageMemoryBarrier> barriers;

	std::shared_ptr<Image> pMotionVector = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[FrameMgr()->FrameIndex()]->GetColorTarget(FrameBufferDiction::MotionVector);
	std::shared_ptr<Image> pShadingResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[FrameMgr()->FrameIndex()]->GetColorTarget(0);
	std::shared_ptr<Image> pSSRResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Shading)[FrameMgr()->FrameIndex()]->GetColorTarget(1);
	std::shared_ptr<Image> pGBuffer1 = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[FrameMgr()->FrameIndex()]->GetColorTarget(FrameBufferDiction::GBuffer1);
	std::shared_ptr<Image> pMotionNeighborMax = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_MotionNeighborMax)[FrameMgr()->FrameIndex()]->GetColorTarget(0);
	std::shared_ptr<Image> pTemporalShadingResult = FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::ShadingResult);
	std::shared_ptr<Image> pTemporalSSRResult = FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::SSRResult);
	std::shared_ptr<Image> pTemporalResult = FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::CombinedResult);
	std::shared_ptr<Image> pCoC = FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, pingpong)->GetColorTarget(FrameBufferDiction::CoC);

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pMotionVector->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pMotionVector->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier = {};
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
	subresourceRange.levelCount = pShadingResult->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pShadingResult->GetImageInfo().arrayLayers;

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
	subresourceRange.levelCount = pGBuffer1->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pGBuffer1->GetImageInfo().arrayLayers;

	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pGBuffer1->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pMotionNeighborMax->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pMotionNeighborMax->GetImageInfo().arrayLayers;

	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pMotionNeighborMax->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pTemporalShadingResult->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pTemporalShadingResult->GetImageInfo().arrayLayers;

	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pTemporalShadingResult->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pTemporalSSRResult->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pTemporalSSRResult->GetImageInfo().arrayLayers;

	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pTemporalSSRResult->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pTemporalResult->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pTemporalResult->GetImageInfo().arrayLayers;

	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pTemporalResult->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	barriers.push_back(imgBarrier);

	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pCoC->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pCoC->GetImageInfo().arrayLayers;

	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pCoC->GetDeviceHandle();
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
}