#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class ShadowMapPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<ShadowMapPass>& pSelf);

public:
	static std::shared_ptr<ShadowMapPass> Create();

public:
	std::vector<VkClearValue> GetClearValue() override;
};