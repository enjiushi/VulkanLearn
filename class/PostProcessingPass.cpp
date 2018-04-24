#include "PostProcessingPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/CommandBuffer.h"

bool PostProcessingPass::Init(const std::shared_ptr<PostProcessingPass>& pSelf)
{
	std::vector<VkAttachmentDescription> attachmentDescs(1);

	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescs[0].format = GetDevice()->GetPhysicalDevice()->GetSurfaceFormat().format;
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;

	std::vector<VkAttachmentReference> postProcessPassColorAttach(1);
	postProcessPassColorAttach[0].attachment = 0;
	postProcessPassColorAttach[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription postProcessSubPass = {};
	postProcessSubPass.colorAttachmentCount = postProcessPassColorAttach.size();
	postProcessSubPass.pColorAttachments = postProcessPassColorAttach.data();
	postProcessSubPass.pDepthStencilAttachment = nullptr;
	postProcessSubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	std::vector<VkSubpassDependency> dependencies(2);

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcAccessMask = 0;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// This one can be generated implicitly without definition
	dependencies[1].srcSubpass = 1;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = 0;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	std::vector<VkSubpassDescription> subPasses = { postProcessSubPass };

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

std::shared_ptr<PostProcessingPass> PostProcessingPass::Create()
{
	std::shared_ptr<PostProcessingPass> pPostProcessingPass = std::make_shared<PostProcessingPass>();
	if (pPostProcessingPass != nullptr && pPostProcessingPass->Init(pPostProcessingPass))
		return pPostProcessingPass;
	return nullptr;
}

std::vector<VkClearValue> PostProcessingPass::GetClearValue()
{
	return
	{
		{ 0.0f, 0.0f, 0.0f, 0.0f }
	};
}