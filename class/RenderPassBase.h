#pragma once

#include "../Base/Base.h"
#include "vulkan.h"
#include "FrameBufferDiction.h"

class RenderPass;
class CommandBuffer;
class FrameBuffer;
class Texture2D;
class DepthStencilBuffer;

class RenderPassBase : public SelfRefBase<RenderPassBase>
{
protected:
	bool Init(const std::shared_ptr<RenderPassBase>& pSelf, const VkRenderPassCreateInfo& info);

public:
	std::shared_ptr<RenderPass> GetRenderPass() const { return m_pRenderPass; }
	uint32_t GetCurrentSubpassIndex() const { return m_currentSubpassIndex; }

	virtual void BeginRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer);
	virtual void EndRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf);
	virtual void NextSubpass(const std::shared_ptr<CommandBuffer>& pCmdBuf);
	virtual std::vector<VkClearValue> GetClearValue() = 0;

protected:
	std::shared_ptr<RenderPass>					m_pRenderPass;
	uint32_t									m_currentSubpassIndex = 0;
};