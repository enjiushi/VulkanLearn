#pragma once
#include "Material.h"
#include "FrameBufferDiction.h"

class ForwardMaterial : public Material
{
public:
	static std::shared_ptr<ForwardMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	void Dispatch(const std::shared_ptr<CommandBuffer>& pCmdBuf, const Vector3f& groupNum, const Vector3f& groupSize, uint32_t pingpong = 0) override {}

protected:
	FrameBufferDiction::FrameBufferType	m_frameBufferType;
};