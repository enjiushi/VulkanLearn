#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class DeferredShadingPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<DeferredShadingPass>& pSelf, VkFormat format, VkImageLayout layout);

	void InitFrameBuffers() override;

public:
	static std::shared_ptr<DeferredShadingPass> Create(VkFormat format, VkImageLayout layout);

public:
	std::vector<VkClearValue> GetClearValue() override;
};