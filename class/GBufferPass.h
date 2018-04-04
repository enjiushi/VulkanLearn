#pragma once

#include "RenderPassBase.h"

class CommandBuffer;

class GBufferPass : public RenderPassBase
{
public:
	enum GBuffer
	{
		GBuffer0,
		GBuffer1,
		GBuffer2,
		GBufferCount
	};

protected:
	bool Init(const std::shared_ptr<GBufferPass>& pSelf);

	void InitFrameBuffers() override;

public:
	static std::shared_ptr<GBufferPass> Create();

public:
	std::vector<VkClearValue> GetClearValue() override;
};