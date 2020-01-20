#pragma once
#include "Material.h"
#include "PerFrameData.h"

class RenderPassBase;
class ShaderStorageBuffer;

class GBufferPlanetMaterial : public Material
{
public:
	static std::shared_ptr<GBufferPlanetMaterial> CreateDefaultMaterial();

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0, bool overrideVP = false) override
	{
		DrawIndirect(pCmdBuf, pFrameBuffer, pingpong, overrideVP);
	}

protected:
	void CustomizeCommandBuffer(const std::shared_ptr<CommandBuffer>& pSecondaryCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0) override;
};