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
	void SetModelMatrix(const Matrix4f& modelMatrix);
	Matrix4f GetViewMatrix() const { return m_perObjectVariables.modelMatrix; }
	Matrix4f GetMVPN() const { return m_perObjectVariables.MVPN; }

	UniformVarList PrepareUniformVarList() override;

protected:
	void SyncBufferDataInternal() override;
	void SetDirty() override;

protected:
	PerObjectVariables	m_perObjectVariables;
};