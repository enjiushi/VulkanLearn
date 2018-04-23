#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class BloomPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<BloomPass>& pSelf);

public:
	static std::shared_ptr<BloomPass> Create();

public:
	std::vector<VkClearValue> GetClearValue() override;
};