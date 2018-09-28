#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class MotionTilePass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<MotionTilePass>& pSelf);

public:
	static std::shared_ptr<MotionTilePass> Create();

public:
	std::vector<VkClearValue> GetClearValue() override;
};