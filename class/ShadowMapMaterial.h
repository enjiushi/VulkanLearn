#pragma once
#include "Material.h"

class ShadowMapMaterial : public Material
{
public:
	static std::shared_ptr<ShadowMapMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf) override;
};