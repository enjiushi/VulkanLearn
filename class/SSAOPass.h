#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class SSAOPass : public RenderPassBase
{
protected:
	bool Init(const std::shared_ptr<SSAOPass>& pSelf, VkFormat format, VkImageLayout layout);

public:
	static std::shared_ptr<SSAOPass> Create(VkFormat format, VkImageLayout layout);

public:
	std::vector<VkClearValue> GetClearValue() override;
};