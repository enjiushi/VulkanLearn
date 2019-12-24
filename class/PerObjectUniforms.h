#pragma once

#include "../Maths/Matrix.h"
#include "ChunkBasedUniforms.h"

class DescriptorSet;

template <typename T>
class PerObjectVariables
{
public:
	Matrix4x4<T> MV;
	Matrix4x4<T> MVP;			// projection * view * model
	Matrix4x4<T> MV_Rotation_P;	// Model view only has rotation, no translation

	Matrix4x4<T> prevMV;
	Matrix4x4<T> prevMVP;
	Matrix4x4<T> prevMV_Rotation_P;
};

typedef PerObjectVariables<float> PerObjectVariablesf;
typedef PerObjectVariables<double> PerObjectVariablesd;

class PerObjectUniforms : public ChunkBasedUniforms
{
protected:
	bool Init(const std::shared_ptr<PerObjectUniforms>& pSelf);

public:
	static std::shared_ptr<PerObjectUniforms> Create();

public:
	void SetModelMatrix(uint32_t index, const Matrix4d& modelMatrix);
	Matrix4d GetMVMatrix(uint32_t index) const { return m_perObjectVariables[index].MV; }
	Matrix4d GetMVP(uint32_t index) const { return m_perObjectVariables[index].MVP; }

	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void UpdateDirtyChunkInternal(uint32_t index) override;
	const void* AcquireDataPtr() const override { return &m_singlePrecisionPerObjectVariables[0]; }
	uint32_t AcquireDataSize() const override { return sizeof(m_singlePrecisionPerObjectVariables); }

protected:
	PerObjectVariablesd		m_perObjectVariables[MAXIMUM_OBJECTS];
	PerObjectVariablesf		m_singlePrecisionPerObjectVariables[MAXIMUM_OBJECTS];

	std::vector<uint32_t>	m_dirtyChunks;
};