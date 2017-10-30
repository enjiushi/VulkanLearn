#pragma once

#include "UniformDataStorage.h"

typedef struct _GlobalVariables
{
	Matrix4f	projectionMatrix;
	Matrix4f	vulkanNDC;
	Matrix4f	PN;		// vulkanNDC * projectionMatrix
}GlobalVariables;

class GlobalUniforms : public UniformDataStorage
{
public:
	void SetProjectionMatrix(const Matrix4f& proj);
	Matrix4f GetProjectionMatrix() const { return m_globalVariables.projectionMatrix; }
	void SetVulkanNDCMatrix(const Matrix4f& vndc);
	Matrix4f GetVulkanNDCMatrix() const { return m_globalVariables.vulkanNDC; }
	Matrix4f GetNPMatrix() const { return m_globalVariables.PN; }

public:
	bool Init(const std::shared_ptr<GlobalUniforms>& pSelf);
	static std::shared_ptr<GlobalUniforms> Create();

	UniformVarList PrepareUniformVarList() override;

protected:
	void SyncBufferDataInternal() override;
	void SetDirty() override;

protected:
	GlobalVariables					m_globalVariables;
};