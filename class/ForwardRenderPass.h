#pragma once

#include "RenderPassBase.h"

class ForwardRenderPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<ForwardRenderPass>& pSelf, VkFormat format, VkImageLayout layout);

public:
	static std::shared_ptr<ForwardRenderPass> Create(VkFormat format, VkImageLayout layout);

public:
	std::vector<VkClearValue> GetClearValue() override;
};