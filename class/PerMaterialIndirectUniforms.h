#pragma once

#include "../Maths/Matrix.h"
#include "ChunkBasedUniforms.h"

class DescriptorSet;

// You can't directly map "gl_drawID" to the array "PerMaterialIndirectVariables" one by one,
// as there could be more than 1 instance for a mesh, and they all share the same "gl_drawID"
// Considering the fact described above, offset is added here serving as another indirect so that 
// one could get a correct index of "PerMaterialIndirectVariables" with "DrawIDOffset[gl_drawID] + gl_instanceID"
typedef struct _IndirectOffset
{
	uint32_t offset = 0;
}IndirectOffset;

// Variables that could indirect to specific data in shader
typedef struct _PerMaterialIndirectVariables
{
	uint32_t perObjectIndex = 0;
	uint32_t perMaterialIndex = 0;
	uint32_t perMeshIndex = 0;
	uint32_t perAnimationIndex = 0;
}PerMaterialIndirectVariables;

class PerMaterialIndirectOffsetUniforms : public ChunkBasedUniforms
{
public:
	bool Init(const std::shared_ptr<PerMaterialIndirectOffsetUniforms>& pSelf);
	static std::shared_ptr<PerMaterialIndirectOffsetUniforms> Create();

public:
	void SetIndirectOffset(uint32_t drawID, uint32_t indirectOffset) { m_indirectOffsets[drawID].offset = indirectOffset; SetChunkDirty(drawID); }
	uint32_t GetIndirectOffset(uint32_t drawID) const { return m_indirectOffsets[drawID].offset; }

	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void UpdateDirtyChunkInternal(uint32_t index) override;
	const void* AcquireDataPtr() const override { return &m_indirectOffsets[0]; }
	uint32_t AcquireDataSize() const override { return sizeof(m_indirectOffsets); }

protected:
	IndirectOffset	m_indirectOffsets[MAXIMUM_OBJECTS];
};


class PerMaterialIndirectUniforms : public ChunkBasedUniforms
{
public:
	bool Init(const std::shared_ptr<PerMaterialIndirectUniforms>& pSelf);
	static std::shared_ptr<PerMaterialIndirectUniforms> Create();

public:
	void SetPerObjectIndex(uint32_t indirectIndex, uint32_t perObjectIndex) { m_perMaterialIndirectIndex[indirectIndex].perObjectIndex = perObjectIndex; SetChunkDirty(indirectIndex); }
	uint32_t GetPerObjectIndex(uint32_t indirectIndex) const { return m_perMaterialIndirectIndex[indirectIndex].perObjectIndex; }
	void SetPerMaterialIndex(uint32_t indirectIndex, uint32_t perMaterialIndex) { m_perMaterialIndirectIndex[indirectIndex].perMaterialIndex = perMaterialIndex; SetChunkDirty(indirectIndex); }
	uint32_t GetPerMaterialIndex(uint32_t indirectIndex) const { return m_perMaterialIndirectIndex[indirectIndex].perMaterialIndex; }
	void SetPerMeshIndex(uint32_t indirectIndex, uint32_t perMeshIndex) { m_perMaterialIndirectIndex[indirectIndex].perMeshIndex = perMeshIndex; SetChunkDirty(indirectIndex); }
	uint32_t GetPerMeshIndex(uint32_t indirectIndex) const { return m_perMaterialIndirectIndex[indirectIndex].perMeshIndex; }
	void SetPerAnimationIndex(uint32_t indirectIndex, uint32_t perAnimationIndex) { m_perMaterialIndirectIndex[indirectIndex].perAnimationIndex = perAnimationIndex; SetChunkDirty(indirectIndex); }
	uint32_t GetPerAnimationindex(uint32_t indirectIndex) const { return m_perMaterialIndirectIndex[indirectIndex].perAnimationIndex; }

	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void UpdateDirtyChunkInternal(uint32_t index) override;
	const void* AcquireDataPtr() const override { return &m_perMaterialIndirectIndex[0]; }
	uint32_t AcquireDataSize() const override { return sizeof(m_perMaterialIndirectIndex); }

protected:
	PerMaterialIndirectVariables	m_perMaterialIndirectIndex[MAXIMUM_OBJECTS];
};