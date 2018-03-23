#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class DeferredRenderPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<DeferredRenderPass>& pSelf, VkFormat format, VkImageLayout layout);

public:
	static std::shared_ptr<DeferredRenderPass> Create(VkFormat format, VkImageLayout layout);

public:
	std::vector<VkClearValue> GetClearValue() override;

	void BeginGeometryPass(const std::shared_ptr<CommandBuffer>& pCmdBuf);
	void EndGeometryPass(const std::shared_ptr<CommandBuffer>& pCmdBuf);

	void BeginShadingPass(const std::shared_ptr<CommandBuffer>& pCmdBuf);
	void EndShadingPass(const std::shared_ptr<CommandBuffer>& pCmdBuf);

	void BeginTransparentPass(const std::shared_ptr<CommandBuffer>& pCmdBuf);
	void EndTransparentPass(const std::shared_ptr<CommandBuffer>& pCmdBuf);
};