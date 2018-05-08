#pragma once
#include "Material.h"

class ShadowMapMaterial : public Material
{
public:
	static std::shared_ptr<ShadowMapMaterial> CreateDefaultMaterial();

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer) override;
};