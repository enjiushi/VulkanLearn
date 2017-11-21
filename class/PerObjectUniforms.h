#pragma once

#include "../Maths/Matrix.h"
#include "ChunkBasedUniforms.h"

typedef struct _PerObjectVariables
{
	Matrix4f modelMatrix;
	Matrix4f MVPN;	//vulkanNDC * projection * view * model
}PerObjectVariables;


class PerObjectUniforms : public ChunkBasedUniforms
{
public:
	bool Init(const std::shared_ptr<PerObjectUniforms>& pSelf);
	static std::shared_ptr<PerObjectUniforms> Create();

public:
	void SetModelMatrix(uint32_t index, const Matrix4f& modelMatrix);
	Matrix4f GetModelMatrix(uint32_t index) const { return m_perObjectVariables[index].modelMatrix; }
	Matrix4f GetMVPN(uint32_t index) const { return m_perObjectVariables[index].MVPN; }

	std::vector<UniformVarList> PrepareUniformVarList() override;

protected:
	void SyncBufferDataInternal() override;
	void SetDirty(uint32_t index);

protected:
	PerObjectVariables	m_perObjectVariables[MAXIMUM_OBJECTS];
};