#include "SSAOMaterial.h"
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

std::shared_ptr<SSAOMaterial> SSAOMaterial::CreateDefaultMaterial()
{
	SimpleMaterialCreateInfo simpleMaterialInfo = {};
	simpleMaterialInfo.shaderPaths = { L"../data/shaders/screen_quad_vert_recon_cs_view_ray.vert.spv", L"", L"", L"", L"../data/shaders/ssao_gen.frag.spv", L"" };
	simpleMaterialInfo.vertexFormat = VertexFormatNul;
	simpleMaterialInfo.vertexFormatInMem = VertexFormatNul;
	simpleMaterialInfo.subpassIndex = 0;
	simpleMaterialInfo.frameBufferType = FrameBufferDiction::FrameBufferType_SSAOSSR;
	simpleMaterialInfo.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOSSR);
	simpleMaterialInfo.depthTestEnable = false;
	simpleMaterialInfo.depthWriteEnable = false;

	std::shared_ptr<SSAOMaterial> pSSAOMaterial = std::make_shared<SSAOMaterial>();

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

	if (pSSAOMaterial.get() && pSSAOMaterial->Init(pSSAOMaterial, simpleMaterialInfo.shaderPaths, simpleMaterialInfo.pRenderPass, createInfo, simpleMaterialInfo.materialUniformVars, simpleMaterialInfo.vertexFormat, simpleMaterialInfo.vertexFormatInMem))
		return pSSAOMaterial;

	return nullptr;
}

bool SSAOMaterial::Init(const std::shared_ptr<SSAOMaterial>& pSelf,
	const std::vector<std::wstring>	shaderPaths,
	const std::shared_ptr<RenderPassBase>& pRenderPass,
	const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
	const std::vector<UniformVar>& materialUniformVars,
	uint32_t vertexFormat,
	uint32_t vertexFormatInMem)
{
	VkPushConstantRange pushConstantRange = { VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float) };

	if (!Material::Init(pSelf, shaderPaths, pRenderPass, pipelineCreateInfo, { pushConstantRange }, materialUniformVars, vertexFormat, vertexFormatInMem, false))
		return false;

	std::vector<CombinedImage> gbuffer0;
	std::vector<CombinedImage> gbuffer2;
	std::vector<CombinedImage> depthBuffer;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pGBufferFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];

		gbuffer0.push_back({
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer0),
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer0)->CreateLinearClampToEdgeSampler(),
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer0)->CreateDefaultImageView()
		});

		gbuffer2.push_back({
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer2),
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer2)->CreateLinearClampToEdgeSampler(),
			pGBufferFrameBuffer->GetColorTarget(FrameBufferDiction::GBuffer2)->CreateDefaultImageView()
		});

		depthBuffer.push_back({
			pGBufferFrameBuffer->GetDepthStencilTarget(),
			pGBufferFrameBuffer->GetDepthStencilTarget()->CreateLinearClampToBorderSampler(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK),
			pGBufferFrameBuffer->GetDepthStencilTarget()->CreateDepthSampleImageView()
		});
	}

	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount, gbuffer0);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 1, gbuffer2);
	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 2, depthBuffer);

	uint32_t index;
	UniformData::GetInstance()->GetGlobalTextures()->GetTextureIndex(RGBA8_1024, "BlueNoise", index);
	m_blueNoiseTexIndex = (float)index;

	return true;
}

void SSAOMaterial::CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout)
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
		"GBuffer2",
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
}

void SSAOMaterial::CustomizePoolSize(std::vector<uint32_t>& counts)
{
	counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += (GetSwapChain()->GetSwapChainImageCount() * (FrameBufferDiction::GBufferCount + 1));
}

void SSAOMaterial::CustomizeSecondaryCmd(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong)
{
	pCmdBuf->PushConstants(m_pPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &m_blueNoiseTexIndex);
}

void SSAOMaterial::AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, uint32_t pingpong)
{
	std::shared_ptr<Image> pGBuffer0 = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[FrameMgr()->FrameIndex()]->GetColorTarget(FrameBufferDiction::GBuffer0);
	std::shared_ptr<Image> pGBuffer2 = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[FrameMgr()->FrameIndex()]->GetColorTarget(FrameBufferDiction::GBuffer2);
	std::shared_ptr<Image> pDepth = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[FrameMgr()->FrameIndex()]->GetDepthStencilTarget();

	VkImageSubresourceRange subresourceRange0 = {};
	subresourceRange0.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange0.baseMipLevel = 0;
	subresourceRange0.levelCount = pGBuffer0->GetImageInfo().mipLevels;
	subresourceRange0.layerCount = pGBuffer0->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier0 = {};
	imgBarrier0.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier0.image = pGBuffer0->GetDeviceHandle();
	imgBarrier0.subresourceRange = subresourceRange0;
	imgBarrier0.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier0.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier0.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier0.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkImageSubresourceRange subresourceRange1 = {};
	subresourceRange1.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange1.baseMipLevel = 0;
	subresourceRange1.levelCount = pGBuffer2->GetImageInfo().mipLevels;
	subresourceRange1.layerCount = pGBuffer2->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier1 = {};
	imgBarrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier1.image = pGBuffer2->GetDeviceHandle();
	imgBarrier1.subresourceRange = subresourceRange1;
	imgBarrier1.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier1.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier1.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier1.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkImageSubresourceRange subresourceRange2 = {};
	subresourceRange2.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	subresourceRange2.baseMipLevel = 0;
	subresourceRange2.levelCount = pDepth->GetImageInfo().mipLevels;
	subresourceRange2.layerCount = pDepth->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier2 = {};
	imgBarrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier2.image = pDepth->GetDeviceHandle();
	imgBarrier2.subresourceRange = subresourceRange2;
	imgBarrier2.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier2.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	imgBarrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		{},
		{},
		{ imgBarrier0, imgBarrier1, imgBarrier2 }
	);
}