#pragma once
#include "Material.h"

class ForwardMaterial : public Material
{
public:
	static std::shared_ptr<ForwardMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf) override;
};