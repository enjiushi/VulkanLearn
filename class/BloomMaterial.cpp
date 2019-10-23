#include "BloomMaterial.h"
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

std::shared_ptr<BloomMaterial> BloomMaterial::CreateDefaultMaterial(BloomPass bloomPass, uint32_t iterIndex)
{
	std::wstring fragShaderPath;
	switch (bloomPass)
	{
	case BloomPass_Prefilter:		fragShaderPath = L"../data/shaders/bloom_prefilter.frag.spv"; break;
	case BloomPass_DownSampleBox13: fragShaderPath = L"../data/shaders/bloom_downsamplebox13.frag.spv"; break;
	case BloomPass_UpSampleTent:	fragShaderPath = L"../data/shaders/bloom_upsampletent.frag.spv"; break;
	default:
		ASSERTION(false);
		break;
	}

	SimpleMaterialCreateInfo simpleMaterialInfo = {};
	simpleMaterialInfo.shaderPaths = { L"../data/shaders/screen_quad.vert.spv", L"", L"", L"", fragShaderPath, L"" };
	simpleMaterialInfo.vertexFormat = VertexFormatNul;
	simpleMaterialInfo.vertexFormatInMem = VertexFormatNul;
	simpleMaterialInfo.subpassIndex = 0;
	simpleMaterialInfo.frameBufferType = FrameBufferDiction::FrameBufferType_Bloom;
	simpleMaterialInfo.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom);
	simpleMaterialInfo.depthTestEnable = false;
	simpleMaterialInfo.depthWriteEnable = false;

	std::shared_ptr<BloomMaterial> pBloomMaterial = std::make_shared<BloomMaterial>();

	VkGraphicsPipelineCreateInfo createInfo = {};

	std::vector<VkPipelineColorBlendAttachmentState> blendStatesInfo;
	uint32_t colorTargetCount = FrameBufferDiction::GetInstance()->GetFrameBuffer(simpleMaterialInfo.frameBufferType)->GetColorTargets().size();

	VkBlendFactor dstFactor = (bloomPass == BloomPass_UpSampleTent) ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	for (uint32_t i = 0; i < colorTargetCount; i++)
	{
		blendStatesInfo.push_back(
			{
				VK_FALSE,							// blend enabled

				VK_BLEND_FACTOR_SRC_ALPHA,			// src color blend factor
				dstFactor,							// dst color blend factor
				VK_BLEND_OP_ADD,					// color blend op

				VK_BLEND_FACTOR_ONE,				// src alpha blend factor
				dstFactor,							// dst alpha blend factor
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
	createInfo.renderPass = simpleMaterialInfo.pRenderPass->GetRenderPass()->GetDeviceHandle();
	createInfo.subpass = simpleMaterialInfo.subpassIndex;

	VkPushConstantRange pushConstantRange0 = { VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Vector2f) };

	pBloomMaterial->m_bloomPass = bloomPass;
	pBloomMaterial->m_iterIndex = iterIndex;

	if (pBloomMaterial.get() && pBloomMaterial->Init(pBloomMaterial, simpleMaterialInfo.shaderPaths, simpleMaterialInfo.pRenderPass, createInfo, { pushConstantRange0 }, simpleMaterialInfo.materialUniformVars, simpleMaterialInfo.vertexFormat, simpleMaterialInfo.vertexFormatInMem))
		return pBloomMaterial;

	return nullptr;
}

bool BloomMaterial::Init(const std::shared_ptr<BloomMaterial>& pSelf,
	const std::vector<std::wstring>	shaderPaths,
	const std::shared_ptr<RenderPassBase>& pRenderPass,
	const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
	const std::vector<VkPushConstantRange>& pushConstsRanges,
	const std::vector<UniformVar>& materialUniformVars,
	uint32_t vertexFormat,
	uint32_t vertexFormatInMem)
{
	if (!Material::Init(pSelf, shaderPaths, pRenderPass, pipelineCreateInfo, pushConstsRanges, materialUniformVars, vertexFormat, vertexFormatInMem, false))
		return false;

	std::vector<CombinedImage> srcImgs;

	switch (m_bloomPass)
	{
	case BloomPass_Prefilter:
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pDOFResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, FrameBufferDiction::CombineLayer)[j];

			srcImgs.push_back({
				pDOFResult->GetColorTarget(0),
				pDOFResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pDOFResult->GetColorTarget(0)->CreateDefaultImageView()
				});
		}
		break;
	case BloomPass_DownSampleBox13:
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPrevBloomResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, m_iterIndex)[j];

			srcImgs.push_back({
				pPrevBloomResult->GetColorTarget(0),
				pPrevBloomResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pPrevBloomResult->GetColorTarget(0)->CreateDefaultImageView()
				});
		}
		break;
	case BloomPass_UpSampleTent:
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPrevBloomResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom, m_iterIndex + 1)[j];

			srcImgs.push_back({
				pPrevBloomResult->GetColorTarget(0),
				pPrevBloomResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pPrevBloomResult->GetColorTarget(0)->CreateDefaultImageView()
				});
		}
		break;
	default:
		ASSERTION(false);
		break;
	}

	m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount, srcImgs);

	return true;
}

void BloomMaterial::CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout)
{
	if (m_bloomPass == BloomPass_Prefilter)
	{
		m_materialVariableLayout.push_back(
		{
			CombinedSampler,
			"DOF result",
			{},
			GetSwapChain()->GetSwapChainImageCount()
		});
	}
	else
	{
		m_materialVariableLayout.push_back(
		{
			CombinedSampler,
			"PrevBloomDown(Up)Result",
			{},
			GetSwapChain()->GetSwapChainImageCount()
		});
	}
}

void BloomMaterial::CustomizePoolSize(std::vector<uint32_t>& counts)
{
	counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += (GetSwapChain()->GetSwapChainImageCount());
}

void BloomMaterial::CustomizeSecondaryCmd(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong)
{
	Vector2f size = { (float)pFrameBuffer->GetFramebufferInfo().width, (float)pFrameBuffer->GetFramebufferInfo().height };
	size *= m_bloomPass == BloomPass_UpSampleTent ? 0.5f : 2.0f;
	size = { 1.0f / size.x, 1.0f / size.y };

	pCmdBuf->PushConstants(m_pPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Vector2f), &size);
}

void BloomMaterial::AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, uint32_t pingpong)
{
	std::shared_ptr<Image> pBarrierImg;

	switch (m_bloomPass)
	{
	case BloomPass_Prefilter:
		pBarrierImg = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_DOF, FrameBufferDiction::CombineLayer)->GetColorTarget(0);
		break;
	case BloomPass_DownSampleBox13:
		pBarrierImg = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Bloom, m_iterIndex)->GetColorTarget(0);
		break;
	case BloomPass_UpSampleTent:
		pBarrierImg = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Bloom, m_iterIndex + 1)->GetColorTarget(0);
		break;
	default:
		ASSERTION(false);
		break;
	}

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = pBarrierImg->GetImageInfo().mipLevels;
	subresourceRange.layerCount = pBarrierImg->GetImageInfo().arrayLayers;

	VkImageMemoryBarrier imgBarrier = {};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pBarrierImg->GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		{},
		{},
		{ imgBarrier }
	);
}