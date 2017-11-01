#pragma once

#include "../Maths/Matrix.h"
#include "UniformDataStorage.h"

typedef struct _PerObjectVariables
{
	Matrix4f modelMatrix;
	Matrix4f MVPN;	//vulkanNDC * projection * view * model
}PerObjectVariables;


class PerObjectUniforms : public UniformDataStorage
{
	static const uint32_t MAXIMUM_OBJECTS = 1024;

public:
	bool Init(const std::shared_ptr<PerObjectUniforms>& pSelf);
	static std::shared_ptr<PerObjectUniforms> Create();

public:
	void SetModelMatrix(uint32_t index, const Matrix4f& modelMatrix);
	Matrix4f GetModelMatrix(uint32_t index) const { return m_perObjectVariables[index].modelMatrix; }
	Matrix4f GetMVPN(uint32_t index) const { return m_perObjectVariables[index].MVPN; }

	UniformVarList PrepareUniformVarList() override;

	uint32_t AllocatePerObjectChunk();
	void FreePreObjectChunk(uint32_t index);

protected:
	void SyncBufferDataInternal() override;
	void SetDirty(uint32_t index);

	void InsertIntoFreeChunk(uint32_t index, uint32_t chunkIndex);
	void FreePreObjectChunk(uint32_t index, uint32_t start, uint32_t end);

protected:
	PerObjectVariables	m_perObjectVariables[MAXIMUM_OBJECTS];
	std::vector<std::pair<uint32_t, uint32_t>>	m_freeChunks;
};