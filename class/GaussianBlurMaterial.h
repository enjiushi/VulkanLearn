#pragma once
#include "Material.h"

class GaussianBlurMaterial : public Material
{
public:
	static std::shared_ptr<GaussianBlurMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf) override;

protected:
	bool	m_isVertical = false;
};