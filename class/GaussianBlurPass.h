#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class GaussianBlurPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<GaussianBlurPass>& pSelf, VkFormat format);

public:
	static std::shared_ptr<GaussianBlurPass> Create(VkFormat format);

public:
	std::vector<VkClearValue> GetClearValue() override;
};