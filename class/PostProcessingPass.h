#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class PostProcessingPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<PostProcessingPass>& pSelf, VkFormat format, VkImageLayout layout);

public:
	static std::shared_ptr<PostProcessingPass> Create(VkFormat temporalFormat, VkImageLayout temporalLayout);

public:
	std::vector<VkClearValue> GetClearValue() override;
};