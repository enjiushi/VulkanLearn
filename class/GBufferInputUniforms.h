#pragma once

#include "../Maths/Matrix.h"
#include "UniformBase.h"

class DescriptorSet;
class Texture2D;

class GBufferInputUniforms : public UniformBase
{
public:
	bool Init(const std::shared_ptr<GBufferInputUniforms>& pSelf);
	static std::shared_ptr<GBufferInputUniforms> Create();

public:
	virtual std::vector<UniformVarList> PrepareUniformVarList() override;
	void SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t reservedIndex = 0) const override;
};