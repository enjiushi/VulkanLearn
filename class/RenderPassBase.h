#pragma once

#include "../Base/Base.h"
#include "vulkan.h"

class RenderPass;
class CommandBuffer;
class FrameBuffer;

class RenderPassBase : public SelfRefBase<RenderPassBase>
{
public:
	static const VkFormat OFFSCREEN_COLOR_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
	static const VkFormat OFFSCREEN_HDR_COLOR_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	static const VkFormat OFFSCREEN_DEPTH_STENCIL_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT;
	static const VkFormat GBUFFER0_COLOR_FORMAT = VK_FORMAT_A2R10G10B10_UNORM_PACK32;

protected:
	bool Init(const std::shared_ptr<RenderPassBase>& pSelf, const VkRenderPassCreateInfo& info);

public:
	virtual void BeginRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer);
	virtual void EndRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer);
	virtual std::vector<VkClearValue> GetClearValue() = 0;

protected:
	std::shared_ptr<RenderPass>	m_pRenderPass;
};