#pragma once
#include "Material.h"

class ForwardMaterial : public Material
{
public:
	static std::shared_ptr<ForwardMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo, bool offScreen);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf) override;

protected:
	bool	m_offScreen;
};