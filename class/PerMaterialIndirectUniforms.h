#pragma once

#include "../Maths/Matrix.h"
#include "ChunkBasedUniforms.h"

class DescriptorSet;

typedef struct _PerMaterialIndirectVariables
{
	uint32_t perObjectIndex = 0;
	uint32_t perMaterialIndex = 0;
}PerMaterialIndirectVariables;


class PerMaterialIndirectUniforms : public UniformDataStorage
{
public:
	bool Init(const std::shared_ptr<PerMaterialIndirectUniforms>& pSelf);
	static std::shared_ptr<PerMaterialIndirectUniforms> Create();

public:
	void SetPerObjectIndex(uint32_t indirectIndex, uint32_t perObjectIndex) { m_perMaterialIndirectIndex[indirectIndex].perObjectIndex = perObjectIndex; SetDirty(); }
	uint32_t GetPerObjectIndex(uint32_t indirectIndex) const { return m_perMaterialIndirectIndex[indirectIndex].perObjectIndex; }
	void SetPerMaterialIndex(uint32_t indirectIndex, uint32_t perMaterialIndex) { m_perMaterialIndirectIndex[indirectIndex].perMaterialIndex = perMaterialIndex; SetDirty(); }
	uint32_t GetPerMaterialIndex(uint32_t indirectIndex) const { return m_perMaterialIndirectIndex[indirectIndex].perMaterialIndex; }

	std::vector<UniformVarList> PrepareUniformVarList() override;
	void SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t reservedIndex = 0) const override;

protected:
	void SyncBufferDataInternal() override;

protected:
	PerMaterialIndirectVariables	m_perMaterialIndirectIndex[MAXIMUM_OBJECTS];
};