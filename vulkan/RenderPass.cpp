#include "RenderPass.h"
#include <algorithm>
#include "../thread/ThreadTaskQueue.hpp"
#include "GlobalDeviceObjects.h"
#include "CommandBuffer.h"

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