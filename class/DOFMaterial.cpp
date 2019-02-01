#include "DOFMaterial.h"
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

std::shared_ptr<DOFMaterial> DOFMaterial::CreateDefaultMaterial(DOFPass pass)
{
	std::wstring fragShaderPath;
	switch (pass)
	{
	case DOFPass_Prefilter:		fragShaderPath = L"../data/shaders/dof_prefilter.frag.spv"; break;
	case DOFPass_Blur:			fragShaderPath = L"../data/shaders/dof_blur.frag.spv"; break;
	case DOFPass_Postfilter:	fragShaderPath = L"../data/shaders/dof_postfilter.frag.spv"; break;
	default:
		ASSERTION(false);
		break;
	}

	SimpleMaterialCreateInfo simpleMaterialInfo = {};
	simpleMaterialInfo.shaderPaths = { L"../data/shaders/screen_quad.vert.spv", L"", L"", L"", fragShaderPath, L"" };
	simpleMaterialInfo.vertexFormat = VertexFormatNul;
	simpleMaterialInfo.subpassIndex = 0;
	simpleMaterialInfo.frameBufferType = FrameBufferDiction::FrameBufferType_DOF;
	simpleMaterialInfo.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassDOF);
	simpleMaterialInfo.depthTestEnable = false;
	simpleMaterialInfo.depthWriteEnable = false;

	std::shared_ptr<DOFMaterial> pDOFMaterial = std::make_shared<DOFMaterial>();

	VkGraphicsPipelineCreateInfo createInfo = {};

	std::vector<VkPipelineColorBlendAttachmentState> blendStatesInfo;
	uint32_t colorTargetCount = FrameBufferDiction::GetInstance()->GetFrameBuffer(simpleMaterialInfo.frameBufferType, pass)->GetColorTargets().size();

	for (uint32_t i = 0; i < colorTargetCount; i++)
	{
		blendStatesInfo.push_back(
			{
				VK_FALSE,							// blend enabled

				VK_BLEND_FACTOR_SRC_ALPHA,			// src color blend factor
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,							// dst color blend factor
				VK_BLEND_OP_ADD,					// color blend op

				VK_BLEND_FACTOR_ONE,				// src alpha blend factor
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,							// dst alpha blend factor
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

	VkPushConstantRange pushConstantRange0 = { VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Vector2f) };

	pDOFMaterial->m_DOFPass = pass;

	if (pDOFMaterial.get() && pDOFMaterial->Init(pDOFMaterial, simpleMaterialInfo.shaderPaths, simpleMaterialInfo.pRenderPass, createInfo, { pushConstantRange0 }, simpleMaterialInfo.materialUniformVars, simpleMaterialInfo.vertexFormat))
		return pDOFMaterial;

	return nullptr;
}

bool DOFMaterial::Init(const std::shared_ptr<DOFMaterial>& pSelf,
	const std::vector<std::wstring>	shaderPaths,
	const std::shared_ptr<RenderPassBase>& pRenderPass,
	const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
	const std::vector<VkPushConstantRange>& pushConstsRanges,
	const std::vector<UniformVar>& materialUniformVars,
	uint32_t vertexFormat)
{
	if (!Material::Init(pSelf, shaderPaths, pRenderPass, pipelineCreateInfo, pushConstsRanges, materialUniformVars, vertexFormat))
		return false;

	std::vector<CombinedImage> srcImgs;

	switch (m_DOFPass)
	{
		case DOFPass_Prefilter:
		{
			for (uint32_t j = 0; j < 2; j++)
			{
				std::shared_ptr<FrameBuffer> pTemporalResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_TemporalResolve)[j];

				srcImgs.push_back({
					pTemporalResult->GetColorTarget(0),
					pTemporalResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
					pTemporalResult->GetColorTarget(0)->CreateDefaultImageView()
					});
			}

			std::vector<CombinedImage> gbuffer3;
			for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
			{
				std::shared_ptr<FrameBuffer> pGBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[j];

				gbuffer3.push_back({
					pGBuffer->GetColorTarget(FrameBufferDiction::GBuffer3),
					pGBuffer->GetColorTarget(FrameBufferDiction::GBuffer3)->CreateLinearClampToEdgeSampler(),
					pGBuffer->GetColorTarget(FrameBufferDiction::GBuffer3)->CreateDefaultImageView()
					});
			}
			m_pUniformStorageDescriptorSet->UpdateImages(MaterialUniformStorageTypeCount + 1, gbuffer3);
		}

		break;
	case DOFPass_Blur:
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pPrefilterResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, DOFPass_Prefilter)[j];

			srcImgs.push_back({
				pPrefilterResult->GetColorTarget(0),
				pPrefilterResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pPrefilterResult->GetColorTarget(0)->CreateDefaultImageView()
				});
		}
		break;
	case DOFPass_Postfilter:
		for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
		{
			std::shared_ptr<FrameBuffer> pBlurResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, DOFPass_Blur)[j];

			srcImgs.push_back({
				pBlurResult->GetColorTarget(0),
				pBlurResult->GetColorTarget(0)->CreateLinearClampToEdgeSampler(),
				pBlurResult->GetColorTarget(0)->CreateDefaultImageView()
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

void DOFMaterial::CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout)
{
	if (m_DOFPass == DOFPass_Prefilter)
	{
		m_materialVariableLayout.push_back(
		{
			CombinedSampler,
			"Temporal result",
			{},
			2
		});

		m_materialVariableLayout.push_back(
		{
			CombinedSampler,
			"GBuffer3",
			{},
			GetSwapChain()->GetSwapChainImageCount()
		});
	}
	else
	{
		m_materialVariableLayout.push_back(
		{
			CombinedSampler,
			"Prev DOF pass result",
			{},
			GetSwapChain()->GetSwapChainImageCount()
		});
	}
}

void DOFMaterial::CustomizePoolSize(std::vector<uint32_t>& counts)
{
	if (m_DOFPass == DOFPass_Prefilter)
	{
		counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += (GetSwapChain()->GetSwapChainImageCount() + 2);
	}
	else
	{
		counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += (GetSwapChain()->GetSwapChainImageCount());
	}
	counts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += (GetSwapChain()->GetSwapChainImageCount());
}

void DOFMaterial::Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong)
{
	std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadPerFrameRes()->AllocateSecondaryCommandBuffer();

	pDrawCmdBuffer->StartSecondaryRecording(m_pRenderPass->GetRenderPass(), m_pPipeline->GetInfo().subpass, pFrameBuffer);

	Vector2f size = { (float)pFrameBuffer->GetFramebufferInfo().width, (float)pFrameBuffer->GetFramebufferInfo().height };
	
	//size *= m_bloomPass == BloomPass_UpSampleTent ? 0.5f : 2.0f;
	//size = { 1.0f / size.x, 1.0f / size.y };

	pDrawCmdBuffer->SetViewports({ GetGlobalVulkanStates()->GetViewport() });
	pDrawCmdBuffer->SetScissors({ GetGlobalVulkanStates()->GetScissorRect() });

	BindPipeline(pDrawCmdBuffer);
	BindDescriptorSet(pDrawCmdBuffer);

	//pDrawCmdBuffer->PushConstants(m_pPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Vector2f), &size);

	pDrawCmdBuffer->Draw(3, 1, 0, 0);

	pDrawCmdBuffer->EndSecondaryRecording();

	pCmdBuf->Execute({ pDrawCmdBuffer });
}

void DOFMaterial::AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, uint32_t pingpong)
{
	std::shared_ptr<Image> pBarrierImg;

	switch (m_DOFPass)
	{
	case DOFPass_Prefilter:
		pBarrierImg = FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, (pingpong + 1) % 2)->GetColorTarget(0);
		break;
	case DOFPass_Blur:
		pBarrierImg = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_DOF, DOFPass_Prefilter)->GetColorTarget(0);
		break;
	case DOFPass_Postfilter:
		pBarrierImg = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_DOF, DOFPass_Blur)->GetColorTarget(0);
		break;
	default:
		ASSERTION(false);
		break;
	}

	std::vector<VkImageMemoryBarrier> barriers;

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

	barriers.push_back(imgBarrier);

	if (m_DOFPass == DOFPass_Prefilter)
	{
		std::shared_ptr<Image> pGBuffer3 = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer)->GetColorTarget(FrameBufferDiction::GBuffer3);

		subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = pGBuffer3->GetImageInfo().mipLevels;
		subresourceRange.layerCount = pGBuffer3->GetImageInfo().arrayLayers;

		imgBarrier = {};
		imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imgBarrier.image = pGBuffer3->GetDeviceHandle();
		imgBarrier.subresourceRange = subresourceRange;
		imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		barriers.push_back(imgBarrier);
	}

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		{},
		{},
		barriers
	);
}