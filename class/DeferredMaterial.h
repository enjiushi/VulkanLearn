#pragma once
#include "Material.h"

class DeferredMaterial : public Material
{
public:
	static std::shared_ptr<DeferredMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf) override;
};