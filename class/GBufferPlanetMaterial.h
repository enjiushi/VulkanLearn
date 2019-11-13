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
	void Dispatch(const std::shared_ptr<CommandBuffer>& pCmdBuf, const Vector3f& groupNum, const Vector3f& groupSize, uint32_t pingpong = 0) override {}
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0, bool overrideVP = false) override;

protected:
	void CustomizeSecondaryCmd(const std::shared_ptr<CommandBuffer>& pSecondaryCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0) override;
};