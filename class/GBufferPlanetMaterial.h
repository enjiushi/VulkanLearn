#pragma once
#include "Material.h"

class RenderPassBase;

class GBufferPlanetMaterial : public Material
{
public:
	static std::shared_ptr<GBufferPlanetMaterial> CreateDefaultMaterial();

public:
	void Dispatch(const std::shared_ptr<CommandBuffer>& pCmdBuf, const Vector3f& groupNum, const Vector3f& groupSize, uint32_t pingpong = 0) override {}
};