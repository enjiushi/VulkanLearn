#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/FrameManager.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/Texture2D.h"
#include "../vulkan/DepthStencilBuffer.h"
#include "RenderPassBase.h"

bool RenderPassBase::Init(const std::shared_ptr<RenderPassBase>& pSelf, const VkRenderPassCreateInfo& info)
{
	if (!SelfRefBase<RenderPassBase>::Init(pSelf))
		return false;

	m_pRenderPass = RenderPass::Create(GetDevice(), info);

	return m_pRenderPass != nullptr;
}

void RenderPassBase::BeginRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer)
{
	pCmdBuf->BeginRenderPass(pFrameBuffer, m_pRenderPass, GetClearValue(), true);
}

void RenderPassBase::EndRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf)
{
	pCmdBuf->EndRenderPass();
	m_currentSubpassIndex = 0;
}

void RenderPassBase::NextSubpass(const std::shared_ptr<CommandBuffer>& pCmdBuf)
{
	pCmdBuf->NextSubpass();
	m_currentSubpassIndex++;
}