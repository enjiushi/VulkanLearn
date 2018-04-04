#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class SSAOPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<SSAOPass>& pSelf);

	void InitFrameBuffers() override {}

public:
	static std::shared_ptr<SSAOPass> Create();

public:
	std::vector<VkClearValue> GetClearValue() override;
};