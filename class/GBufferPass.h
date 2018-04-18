#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class GBufferPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<GBufferPass>& pSelf);

public:
	static std::shared_ptr<GBufferPass> Create();

public:
	std::vector<VkClearValue> GetClearValue() override;
};