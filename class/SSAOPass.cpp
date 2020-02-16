#include "SSAOPass.h"
#include "RenderPassDiction.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/Image.h"

bool SSAOPass::Init(const std::shared_ptr<SSAOPass>& pSelf, VkFormat format, VkImageLayout layout)
{
	std::vector<VkAttachmentDescription> attachmentDescs(1);

	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[0].finalLayout = layout;
	attachmentDescs[0].format = format;
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;

	std::vector<VkAttachmentReference> ssaoPassColorAttach(1);
	ssaoPassColorAttach[0].attachment = 0;
	ssaoPassColorAttach[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription ssaoSubPass = {};
	ssaoSubPass.colorAttachmentCount = (uint32_t)ssaoPassColorAttach.size();
	ssaoSubPass.pColorAttachments = ssaoPassColorAttach.data();
	ssaoSubPass.pDepthStencilAttachment = nullptr;
	ssaoSubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	std::vector<VkSubpassDependency> dependencies(2);

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcAccessMask = 0;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// This one can be generated implicitly without definition
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = 0;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	std::vector<VkSubpassDescription> subPasses = { ssaoSubPass };

	VkRenderPassCreateInfo renderpassCreateInfo = {};
	renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.attachmentCount = (uint32_t)attachmentDescs.size();
	renderpassCreateInfo.pAttachments = attachmentDescs.data();
	renderpassCreateInfo.subpassCount = (uint32_t)subPasses.size();
	renderpassCreateInfo.pSubpasses = subPasses.data();
	renderpassCreateInfo.dependencyCount = (uint32_t)dependencies.size();
	renderpassCreateInfo.pDependencies = dependencies.data();

	if (!RenderPassBase::Init(pSelf, renderpassCreateInfo))
		return false;
	return true;
}

std::shared_ptr<SSAOPass> SSAOPass::Create(VkFormat format, VkImageLayout layout)
{
	std::shared_ptr<SSAOPass> pSSAOPass = std::make_shared<SSAOPass>();
	if (pSSAOPass != nullptr && pSSAOPass->Init(pSSAOPass, format, layout))
		return pSSAOPass;
	return nullptr;
}

std::vector<VkClearValue> SSAOPass::GetClearValue()
{
	return
	{
		{ 1.0f }
	};
}