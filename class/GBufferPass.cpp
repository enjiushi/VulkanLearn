#include "GBufferPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/Texture2D.h"
#include "../vulkan/DepthStencilBuffer.h"
#include "../vulkan/CommandBuffer.h"
#include "FrameBufferDiction.h"

bool GBufferPass::Init(const std::shared_ptr<GBufferPass>& pSelf)
{
	std::vector<VkAttachmentDescription> attachmentDescs(6);

	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDescs[0].format = FrameBufferDiction::GetGBufferFormat(FrameBufferDiction::GBuffer0);
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDescs[1].format = FrameBufferDiction::GetGBufferFormat(FrameBufferDiction::GBuffer1);
	attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDescs[2].format = FrameBufferDiction::GetGBufferFormat(FrameBufferDiction::GBuffer2);
	attachmentDescs[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[2].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[3].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDescs[3].format = FrameBufferDiction::GetGBufferFormat(FrameBufferDiction::GBuffer3);
	attachmentDescs[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[3].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[4].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDescs[4].format = FrameBufferDiction::GetGBufferFormat(FrameBufferDiction::MotionVector);
	attachmentDescs[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[4].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[5].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[5].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDescs[5].format = FrameBufferDiction::OFFSCREEN_DEPTH_STENCIL_FORMAT;
	attachmentDescs[5].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[5].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[5].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[5].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[5].samples = VK_SAMPLE_COUNT_1_BIT;

	std::vector<VkAttachmentReference> GBufferPassColorAttach(5);
	GBufferPassColorAttach[0].attachment = 0;
	GBufferPassColorAttach[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	GBufferPassColorAttach[1].attachment = 1;
	GBufferPassColorAttach[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	GBufferPassColorAttach[2].attachment = 2;
	GBufferPassColorAttach[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	GBufferPassColorAttach[3].attachment = 3;
	GBufferPassColorAttach[3].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	GBufferPassColorAttach[4].attachment = 4;
	GBufferPassColorAttach[4].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachment = {};
	depthAttachment.attachment = 5;
	depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription GBufferSubPass = {};
	GBufferSubPass.colorAttachmentCount = GBufferPassColorAttach.size();
	GBufferSubPass.pColorAttachments = GBufferPassColorAttach.data();
	GBufferSubPass.pDepthStencilAttachment = &depthAttachment;
	GBufferSubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	std::vector<VkAttachmentReference> backgroundMotionAttach(1);
	backgroundMotionAttach[0].attachment = 4;
	backgroundMotionAttach[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription backgroundMotionSubPass = {};
	backgroundMotionSubPass.colorAttachmentCount = backgroundMotionAttach.size();
	backgroundMotionSubPass.pColorAttachments = backgroundMotionAttach.data();
	backgroundMotionSubPass.pDepthStencilAttachment = &depthAttachment;
	backgroundMotionSubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

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
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// This one can be generated implicitly without definition
	dependencies[2].srcSubpass = 0;
	dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[2].dstAccessMask = 0;
	dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	//dependencies[1].srcSubpass = 0;
	//dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	//dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//dependencies[1].dstAccessMask = 0;
	//dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	//dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	std::vector<VkSubpassDescription> subPasses = { GBufferSubPass, backgroundMotionSubPass };

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

std::shared_ptr<GBufferPass> GBufferPass::Create()
{
	std::shared_ptr<GBufferPass> pDeferredRenderPass = std::make_shared<GBufferPass>();
	if (pDeferredRenderPass != nullptr && pDeferredRenderPass->Init(pDeferredRenderPass))
		return pDeferredRenderPass;
	return nullptr;
}

std::vector<VkClearValue> GBufferPass::GetClearValue()
{
	return
	{
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 0 }
	};
}