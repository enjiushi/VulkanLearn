#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class PostProcessingPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<PostProcessingPass>& pSelf);

	void InitFrameBuffers() override {}

public:
	static std::shared_ptr<PostProcessingPass> Create();

public:
	std::vector<VkClearValue> GetClearValue() override;
};