#include "DeferredShadingPass.h"
#include "RenderPassDiction.h"
#include "GBufferPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/Texture2D.h"
#include "../vulkan/CommandBuffer.h"
#include "FrameBufferDiction.h"

bool DeferredShadingPass::Init(const std::shared_ptr<DeferredShadingPass>& pSelf, VkFormat format, VkImageLayout layout)
{
	std::vector<VkAttachmentDescription> attachmentDescs(3);

	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[0].finalLayout = layout;
	attachmentDescs[0].format = format;
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[1].finalLayout = layout;
	attachmentDescs[1].format = format;
	attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[2].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDescs[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDescs[2].format = FrameBufferDiction::OFFSCREEN_DEPTH_STENCIL_FORMAT;
	attachmentDescs[2].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachmentDescs[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[2].samples = VK_SAMPLE_COUNT_1_BIT;

	std::vector<VkAttachmentReference> shadingPassColorAttach(2);
	shadingPassColorAttach[0].attachment = 0;
	shadingPassColorAttach[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	shadingPassColorAttach[1].attachment = 1;
	shadingPassColorAttach[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription shadingSubPass = {};
	shadingSubPass.colorAttachmentCount = shadingPassColorAttach.size();
	shadingSubPass.pColorAttachments = shadingPassColorAttach.data();
	shadingSubPass.pDepthStencilAttachment = nullptr;
	shadingSubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkAttachmentReference depthAttachment = {};
	depthAttachment.attachment = 2;
	depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkSubpassDescription skyBoxSubPass = {};
	skyBoxSubPass.colorAttachmentCount = shadingPassColorAttach.size();
	skyBoxSubPass.pColorAttachments = shadingPassColorAttach.data();
	skyBoxSubPass.pDepthStencilAttachment = &depthAttachment;
	skyBoxSubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	std::vector<VkSubpassDependency> dependencies(3);

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
	dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// This one can be generated implicitly without definition
	dependencies[2].srcSubpass = 1;
	dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[2].dstAccessMask = 0;
	dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	std::vector<VkSubpassDescription> subPasses = { shadingSubPass, skyBoxSubPass };

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

std::shared_ptr<DeferredShadingPass> DeferredShadingPass::Create(VkFormat format, VkImageLayout layout)
{
	std::shared_ptr<DeferredShadingPass> pDeferredShadingPass = std::make_shared<DeferredShadingPass>();
	if (pDeferredShadingPass != nullptr && pDeferredShadingPass->Init(pDeferredShadingPass, format, layout))
		return pDeferredShadingPass;
	return nullptr;
}

std::vector<VkClearValue> DeferredShadingPass::GetClearValue()
{
	return
	{
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0 }
	};
}