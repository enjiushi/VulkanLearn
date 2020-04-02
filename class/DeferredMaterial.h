#pragma once
#include "Material.h"

class RenderPassBase;

class GBufferMaterial : public Material
{
public:
	static std::shared_ptr<GBufferMaterial> CreateDefaultMaterial(bool skinned = false);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0, bool overrideVP = false) override
	{
		DrawIndirect(pCmdBuf, pFrameBuffer, pingpong, overrideVP);
	}
};