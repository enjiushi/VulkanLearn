#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class TemporalPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<TemporalPass>& pSelf, VkFormat format, VkImageLayout layout);

public:
	static std::shared_ptr<TemporalPass> Create(VkFormat format, VkImageLayout layout);

public:
	std::vector<VkClearValue> GetClearValue() override;
};