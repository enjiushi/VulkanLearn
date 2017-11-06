#pragma once

#include "UniformDataStorage.h"

typedef struct _GlobalVariables
{
	// Camera settings
	Matrix4f	projectionMatrix;
	Matrix4f	vulkanNDC;
	Matrix4f	PN;		// vulkanNDC * projectionMatrix

	// Scene settings
	Vector4f	mainLightDir;		// Main directional light		w: empty for padding
	Vector4f	mainLightColor;		// Main directional light color	w: empty for padding

	// Render settings
	Vector4f	GEW;	// x: Gamma, y: Exposure, z: White Scale, w: empty for padding
}GlobalVariables;

class GlobalUniforms : public UniformDataStorage
{
public:
	void SetProjectionMatrix(const Matrix4f& proj);
	Matrix4f GetProjectionMatrix() const { return m_globalVariables.projectionMatrix; }
	void SetVulkanNDCMatrix(const Matrix4f& vndc);
	Matrix4f GetVulkanNDCMatrix() const { return m_globalVariables.vulkanNDC; }
	Matrix4f GetPNMatrix() const { return m_globalVariables.PN; }

	void SetMainLightDir(const Vector3f& dir);
	Vector4f GetMainLightDir() const { return m_globalVariables.mainLightDir; }
	void SetMainLightColor(const Vector3f& color);
	Vector4f GetMainLightColor() const { return m_globalVariables.mainLightColor; }

	void SetRenderSettings(const Vector4f& setting);
	Vector4f GetRenderSettings() const { return m_globalVariables.GEW; }

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