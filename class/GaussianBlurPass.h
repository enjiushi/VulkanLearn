#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class GaussianBlurPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<GaussianBlurPass>& pSelf);

public:
	static std::shared_ptr<GaussianBlurPass> Create();

public:
	std::vector<VkClearValue> GetClearValue() override;
};