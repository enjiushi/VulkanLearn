#include "RenderWorkManager.h"
#include "GlobalDeviceObjects.h"
#include "RenderPass.h"
#include "Framebuffer.h"

std::shared_ptr<RenderPass>	RenderWorkManager::m_pDefaultRenderPass = nullptr;
std::shared_ptr<RenderPass>	RenderWorkManager::m_pDefaultOffScreenRenderPass = nullptr;

std::shared_ptr<RenderWorkManager> RenderWorkManager::Create(const std::shared_ptr<Device>& pDevice)
{
	CreateDefaultRenderPass();

	std::shared_ptr<RenderWorkManager> pRenderWorkMgr = std::make_shared<RenderWorkManager>();
	if (pRenderWorkMgr.get() && pRenderWorkMgr->Init(pDevice, pRenderWorkMgr))
		return pRenderWorkMgr;
	return nullptr;
}

bool RenderWorkManager::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<RenderWorkManager>& pSelf)
{
	if (!DeviceObjectBase<RenderWorkManager>::Init(pDevice, pSelf))
		return false;

	return true;
}

bool RenderWorkManager::CreateDefaultRenderPass()
{
	if (m_pDefaultRenderPass != nullptr)
		return true;

	std::vector<VkAttachmentDescription> attachmentDescs(2);
	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescs[0].format = ::GetDevice()->GetPhysicalDevice()->GetSurfaceFormat().format;
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs[1].format = ::GetDevice()->GetPhysicalDevice()->GetDepthStencilFormat();
	attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference colorAttach = {};
	colorAttach.attachment = 0;
	colorAttach.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference dsAttach = {};
	dsAttach.attachment = 1;
	dsAttach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttach;
	subpass.pDepthStencilAttachment = &dsAttach;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstSubpass = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderpassCreateInfo = {};
	renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.attachmentCount = attachmentDescs.size();
	renderpassCreateInfo.pAttachments = attachmentDescs.data();
	renderpassCreateInfo.subpassCount = 1;
	renderpassCreateInfo.pSubpasses = &subpass;
	renderpassCreateInfo.dependencyCount = 1;
	renderpassCreateInfo.pDependencies = &subpassDependency;

	m_pDefaultRenderPass = RenderPass::Create(::GetDevice(), renderpassCreateInfo);
	return m_pDefaultRenderPass != nullptr;
}

std::shared_ptr<RenderPass> RenderWorkManager::GetDefaultRenderPass()
{
	ASSERTION(CreateDefaultRenderPass());
	return m_pDefaultRenderPass;
}

bool RenderWorkManager::CreateDefaultOffscreenRenderPass()
{
	if (m_pDefaultOffScreenRenderPass != nullptr)
		return true;

	std::vector<VkAttachmentDescription> attachmentDescs(2);
	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDescs[0].format = FrameBuffer::OFFSCREEN_HDR_COLOR_FORMAT;
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs[1].format = FrameBuffer::OFFSCREEN_DEPTH_STENCIL_FORMAT;
	attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference colorAttach = {};
	colorAttach.attachment = 0;
	colorAttach.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference dsAttach = {};
	dsAttach.attachment = 1;
	dsAttach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttach;
	subpass.pDepthStencilAttachment = &dsAttach;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstSubpass = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderpassCreateInfo = {};
	renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.attachmentCount = attachmentDescs.size();
	renderpassCreateInfo.pAttachments = attachmentDescs.data();
	renderpassCreateInfo.subpassCount = 1;
	renderpassCreateInfo.pSubpasses = &subpass;
	renderpassCreateInfo.dependencyCount = 1;
	renderpassCreateInfo.pDependencies = &subpassDependency;

	m_pDefaultOffScreenRenderPass = RenderPass::Create(::GetDevice(), renderpassCreateInfo);
	return m_pDefaultOffScreenRenderPass != nullptr;
}

std::shared_ptr<RenderPass> RenderWorkManager::GetDefaultOffscreenRenderPass()
{
	ASSERTION(CreateDefaultOffscreenRenderPass());
	return m_pDefaultOffScreenRenderPass;
}