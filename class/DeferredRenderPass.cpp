#include "DeferredRenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/CommandBuffer.h"

bool DeferredRenderPass::Init(const std::shared_ptr<DeferredRenderPass>& pSelf, VkFormat format, VkImageLayout layout)
{
	std::vector<VkAttachmentDescription> attachmentDescs(5);

	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[0].finalLayout = layout;
	attachmentDescs[0].format = format;
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDescs[1].format = GBUFFER0_COLOR_FORMAT;
	attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDescs[2].format = OFFSCREEN_COLOR_FORMAT;
	attachmentDescs[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[2].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[3].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDescs[3].format = OFFSCREEN_COLOR_FORMAT;
	attachmentDescs[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[3].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs[4].format = OFFSCREEN_DEPTH_STENCIL_FORMAT;
	attachmentDescs[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[4].samples = VK_SAMPLE_COUNT_1_BIT;

	std::vector<VkAttachmentReference> GBufferPassColorAttach(4);
	GBufferPassColorAttach[0].attachment = 0;
	GBufferPassColorAttach[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	GBufferPassColorAttach[1].attachment = 1;
	GBufferPassColorAttach[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	GBufferPassColorAttach[2].attachment = 2;
	GBufferPassColorAttach[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	GBufferPassColorAttach[3].attachment = 3;
	GBufferPassColorAttach[3].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference shadingPassColorAttach = {};
	shadingPassColorAttach.attachment = 0;
	shadingPassColorAttach.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference GBufferPassDSAttach = {};
	GBufferPassDSAttach.attachment = 4;
	GBufferPassDSAttach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription GBufferSubPass = {};
	GBufferSubPass.colorAttachmentCount = GBufferPassColorAttach.size();
	GBufferSubPass.pColorAttachments = GBufferPassColorAttach.data();
	GBufferSubPass.pDepthStencilAttachment = &GBufferPassDSAttach;
	GBufferSubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	std::vector<VkAttachmentReference> shadingPassGBufferInput(3);
	shadingPassGBufferInput[0].attachment = 1;
	shadingPassGBufferInput[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	shadingPassGBufferInput[1].attachment = 2;
	shadingPassGBufferInput[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	shadingPassGBufferInput[2].attachment = 3;
	shadingPassGBufferInput[2].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkSubpassDescription shadingSubPass = {};
	shadingSubPass.colorAttachmentCount = 1;
	shadingSubPass.pColorAttachments = &shadingPassColorAttach;
	shadingSubPass.pDepthStencilAttachment = &GBufferPassDSAttach;
	shadingSubPass.inputAttachmentCount = shadingPassGBufferInput.size();
	shadingSubPass.pInputAttachments = shadingPassGBufferInput.data();
	shadingSubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkSubpassDescription transparentgSubPass = {};
	transparentgSubPass.colorAttachmentCount = 1;
	transparentgSubPass.pColorAttachments = &shadingPassColorAttach;
	transparentgSubPass.pDepthStencilAttachment = &GBufferPassDSAttach;
	transparentgSubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	std::vector<VkSubpassDependency> dependencies(4);

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcAccessMask = 0;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = 1;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[2].srcSubpass = 1;
	dependencies[2].dstSubpass = 2;
	dependencies[2].srcAccessMask = 0;
	dependencies[2].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// This one can be generated implicitly without definition
	dependencies[3].srcSubpass = 2;
	dependencies[3].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[3].dstAccessMask = 0;
	dependencies[3].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[3].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	std::vector<VkSubpassDescription> subPasses = { GBufferSubPass, shadingSubPass, transparentgSubPass };

	VkRenderPassCreateInfo renderpassCreateInfo = {};
	renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.attachmentCount = attachmentDescs.size();
	renderpassCreateInfo.pAttachments = attachmentDescs.data();
	renderpassCreateInfo.subpassCount = subPasses.size();
	renderpassCreateInfo.pSubpasses = subPasses.data();
	renderpassCreateInfo.dependencyCount = dependencies.size();
	renderpassCreateInfo.pDependencies = dependencies.data();

	if (!RenderPassBase::Init(pSelf, renderpassCreateInfo))
		return false;
	return true;
}

std::shared_ptr<DeferredRenderPass> DeferredRenderPass::Create(VkFormat format, VkImageLayout layout)
{
	std::shared_ptr<DeferredRenderPass> pDeferredRenderPass = std::make_shared<DeferredRenderPass>();
	if (pDeferredRenderPass != nullptr && pDeferredRenderPass->Init(pDeferredRenderPass, format, layout))
		return pDeferredRenderPass;
	return nullptr;
}

std::vector<VkClearValue> DeferredRenderPass::GetClearValue()
{
	return
	{
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0 }
	};
}

void DeferredRenderPass::BeginGeometryPass(const std::shared_ptr<CommandBuffer>& pCmdBuf)
{

}

void DeferredRenderPass::EndGeometryPass(const std::shared_ptr<CommandBuffer>& pCmdBuf)
{

}

void DeferredRenderPass::BeginShadingPass(const std::shared_ptr<CommandBuffer>& pCmdBuf)
{
	pCmdBuf->NextSubpass();
}

void DeferredRenderPass::EndShadingPass(const std::shared_ptr<CommandBuffer>& pCmdBuf)
{

}

void DeferredRenderPass::BeginTransparentPass(const std::shared_ptr<CommandBuffer>& pCmdBuf)
{
	pCmdBuf->NextSubpass();
}

void DeferredRenderPass::EndTransparentPass(const std::shared_ptr<CommandBuffer>& pCmdBuf)
{

}
