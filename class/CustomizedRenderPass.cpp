#include "CustomizedRenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/Texture2D.h"
#include "../vulkan/DepthStencilBuffer.h"
#include "../vulkan/CommandBuffer.h"
#include "FrameBufferDiction.h"

bool CustomizedRenderPass::Init(const std::shared_ptr<CustomizedRenderPass>& pSelf, const std::vector<RenderPassAttachDesc>& attachList)
{
	std::vector<VkAttachmentDescription> attachmentDescs(attachList.size());

	std::vector<VkAttachmentReference> colorAttachmentRefs;
	std::vector<VkAttachmentReference> depthStencilAttachmentRefs;

	for (uint32_t i = 0; i < attachList.size(); i++)
	{
		attachmentDescs[i].initialLayout = attachList[i].initialLayout;
		attachmentDescs[i].finalLayout = attachList[i].finalLayout;
		attachmentDescs[i].format = attachList[i].format;
		attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;

		if (attachList[i].format == FrameBufferDiction::OFFSCREEN_DEPTH_STENCIL_FORMAT ||
			attachList[i].format == FrameBufferDiction::OFFSCREEN_DEPTH_FORMAT)
		{
			depthStencilAttachmentRefs.push_back({});
			depthStencilAttachmentRefs[depthStencilAttachmentRefs.size() - 1].attachment = i;
			depthStencilAttachmentRefs[depthStencilAttachmentRefs.size() - 1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			colorAttachmentRefs.push_back({});
			colorAttachmentRefs[colorAttachmentRefs.size() - 1].attachment = i;
			colorAttachmentRefs[colorAttachmentRefs.size() - 1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}

	VkSubpassDescription subPass = {};
	subPass.colorAttachmentCount = (uint32_t)colorAttachmentRefs.size();
	subPass.pColorAttachments = colorAttachmentRefs.data();
	subPass.pDepthStencilAttachment = depthStencilAttachmentRefs.data();
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

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

	std::vector<VkSubpassDescription> subPasses = { subPass };

	VkRenderPassCreateInfo renderpassCreateInfo = {};
	renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.attachmentCount = (uint32_t)attachmentDescs.size();
	renderpassCreateInfo.pAttachments = attachmentDescs.data();
	renderpassCreateInfo.subpassCount = (uint32_t)subPasses.size();
	renderpassCreateInfo.pSubpasses = subPasses.data();
	renderpassCreateInfo.dependencyCount = (uint32_t)dependencies.size();
	renderpassCreateInfo.pDependencies = dependencies.data();

	std::for_each(attachList.begin(), attachList.end(), [&](const RenderPassAttachDesc& v) { m_clearColors.push_back(v.clearValue); });

	if (!RenderPassBase::Init(pSelf, renderpassCreateInfo))
		return false;
	return true;
}

std::shared_ptr<CustomizedRenderPass> CustomizedRenderPass::Create(const std::vector<RenderPassAttachDesc>& attachList)
{
	std::shared_ptr<CustomizedRenderPass> pCustomizedRenderPass = std::make_shared<CustomizedRenderPass>();
	if (pCustomizedRenderPass != nullptr && pCustomizedRenderPass->Init(pCustomizedRenderPass, attachList))
		return pCustomizedRenderPass;
	return nullptr;
}

std::vector<VkClearValue> CustomizedRenderPass::GetClearValue()
{
	return m_clearColors;
}