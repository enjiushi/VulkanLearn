#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/CommandBuffer.h"
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

void RenderPassBase::EndRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer)
{
	pCmdBuf->EndRenderPass();
}
