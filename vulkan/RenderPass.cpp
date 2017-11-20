#include "RenderPass.h"
#include <algorithm>
#include "../thread/ThreadTaskQueue.hpp"
#include "GlobalDeviceObjects.h"
#include "CommandBuffer.h"
#include "Framebuffer.h"

RenderPass::~RenderPass()
{
	vkDestroyRenderPass(m_pDevice->GetDeviceHandle(), m_renderPass, nullptr);
}

std::shared_ptr<RenderPass> RenderPass::Create(const std::shared_ptr<Device>& pDevice, const VkRenderPassCreateInfo& renderPassInfo)
{
	std::shared_ptr<RenderPass> pRenderPass = std::make_shared<RenderPass>();
	if (pRenderPass.get() && pRenderPass->Init(pDevice, pRenderPass, renderPassInfo))
		return pRenderPass;
	return nullptr;
}

bool RenderPass::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<RenderPass>& pSelf, const VkRenderPassCreateInfo& renderPassInfo)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_renderPassInfo = renderPassInfo;

	for (uint32_t i = 0; i < m_renderPassInfo.attachmentCount; i++)
		m_attachmentDescList.push_back(m_renderPassInfo.pAttachments[i]);
	m_renderPassInfo.pAttachments = m_attachmentDescList.data();

	for (uint32_t i = 0; i < m_renderPassInfo.dependencyCount; i++)
		m_subpassDependencyList.push_back(m_renderPassInfo.pDependencies[i]);
	m_renderPassInfo.pDependencies = m_subpassDependencyList.data();

	for (uint32_t i = 0; i < m_renderPassInfo.subpassCount; i++)
	{
		VkSubpassDescription subpassDesc = m_renderPassInfo.pSubpasses[i];
		SubpassDef subpass = {};
		
		for (uint32_t i = 0; i < subpassDesc.colorAttachmentCount; i++)
			subpass.m_colorAttachmentRefs.push_back(subpassDesc.pColorAttachments[i]);
		subpass.m_depthStencilAttachmentRef = *subpassDesc.pDepthStencilAttachment;

		for (uint32_t i = 0; i < subpassDesc.inputAttachmentCount; i++)
			subpass.m_inputAttachmentRefs.push_back(subpassDesc.pInputAttachments[i]);

		for (uint32_t i = 0; i < subpassDesc.preserveAttachmentCount; i++)
			subpass.m_inputAttachmentRefs.push_back(subpassDesc.pInputAttachments[i]);

		if (subpassDesc.pResolveAttachments != nullptr)
		{
			for (uint32_t i = 0; i < subpassDesc.colorAttachmentCount; i++)
				subpass.m_resolveAttachmentRefs.push_back(subpassDesc.pResolveAttachments[i]);
		}
		m_subpasses.push_back(subpass);

		subpassDesc.pColorAttachments = m_subpasses[i].m_colorAttachmentRefs.data();
		subpassDesc.pDepthStencilAttachment = &m_subpasses[i].m_depthStencilAttachmentRef;
		subpassDesc.pInputAttachments = m_subpasses[i].m_inputAttachmentRefs.data();
		subpassDesc.pPreserveAttachments = m_subpasses[i].m_preservAttachmentRefs.data();
		subpassDesc.pResolveAttachments = m_subpasses[i].m_resolveAttachmentRefs.data();
		m_subpassDescList.push_back(subpassDesc);
	}
	m_renderPassInfo.pSubpasses = m_subpassDescList.data();

	CHECK_VK_ERROR(vkCreateRenderPass(pDevice->GetDeviceHandle(), &m_renderPassInfo, nullptr, &m_renderPass));

	return true;
}

void RenderPass::CacheSecondaryCommandBuffer(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	std::unique_lock<std::mutex> lock(m_secBufferMutex);
	m_secondaryCommandBuffers.push_back(pCmdBuffer);
}

void RenderPass::ExecuteCachedSecondaryCommandBuffers(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	GlobalThreadTaskQueue()->WaitForFree();

	std::unique_lock<std::mutex> lock(m_secBufferMutex);

	if (m_secondaryCommandBuffers.size() == 0)
		return;

	pCmdBuffer->ExecuteSecondaryCommandBuffer(m_secondaryCommandBuffers);

	m_secondaryCommandBuffers.clear();
}

std::shared_ptr<RenderPass> RenderPass::CreateDefaultRenderPass()
{
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

	return Create(GlobalDeviceObjects::GetInstance()->GetDevice(), renderpassCreateInfo);
}

std::shared_ptr<RenderPass> RenderPass::CreateDefaultOffscreenRenderPass()
{
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

	return Create(GlobalDeviceObjects::GetInstance()->GetDevice(), renderpassCreateInfo);
}