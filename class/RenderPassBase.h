#pragma once

#include "../Base/Base.h"
#include "vulkan.h"

class RenderPass;
class CommandBuffer;
class FrameBuffer;
class Texture2D;
class DepthStencilBuffer;

class RenderPassBase : public SelfRefBase<RenderPassBase>
{
public:
	static const VkFormat OFFSCREEN_COLOR_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
	static const VkFormat OFFSCREEN_HDR_COLOR_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	static const VkFormat OFFSCREEN_DEPTH_STENCIL_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT;
	static const VkFormat OFFSCREEN_DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;
	static const VkFormat GBUFFER0_COLOR_FORMAT = VK_FORMAT_A2R10G10B10_UNORM_PACK32;

protected:
	bool Init(const std::shared_ptr<RenderPassBase>& pSelf, const VkRenderPassCreateInfo& info);

	virtual void InitFrameBuffers() = 0;

public:
	std::shared_ptr<RenderPass> GetRenderPass() const { return m_pRenderPass; }
	std::shared_ptr<FrameBuffer> GetFrameBuffer();
	std::shared_ptr<FrameBuffer> GetFrameBuffer(uint32_t index) { return m_frameBuffers[index]; }
	uint32_t GetCurrentSubpassIndex() const { return m_currentSubpassIndex; }

	virtual void BeginRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf);
	virtual void EndRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf);
	virtual void NextSubpass(const std::shared_ptr<CommandBuffer>& pCmdBuf);
	virtual std::vector<VkClearValue> GetClearValue() = 0;

protected:
	std::shared_ptr<RenderPass>					m_pRenderPass;
	std::vector<std::shared_ptr<FrameBuffer>>	m_frameBuffers;
	uint32_t									m_currentSubpassIndex = 0;
};