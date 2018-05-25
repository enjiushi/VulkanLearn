#pragma once

#include "UniformDataStorage.h"

class DescriptorSetLayout;
class DescriptorSet;
class GlobalTextures;

const static uint32_t SSAO_SAMPLE_COUNT = 64;

typedef struct _GlobalVariables
{
	// Camera settings
	Matrix4f	projectionMatrix;
	Matrix4f	vulkanNDC;
	Matrix4f	PN;		// vulkanNDC * projectionMatrix

	Matrix4f	prevProjectionMatrix;
	Matrix4f	prevPN;

	// Windows settings
	Vector4f	gameWindowSize;
	Vector4f	envGenWindowSize;
	Vector4f	shadowGenWindowSize;
	Vector4f	SSAOWindowSize;
	Vector4f	bloomWindowSize;

	// Scene settings
	Vector4f	mainLightDir;		// Main directional light		w: empty for padding
	Vector4f	mainLightColor;		// Main directional light color	w: empty for padding
	Matrix4f	mainLightVPN;

	// Render settings
	Vector4f	GEW;	// x: Gamma, y: Exposure, z: White Scale, w: empty for padding

	// SSAO settings
	Vector4f	SSAOSamples[SSAO_SAMPLE_COUNT];
}GlobalVariables;

class GlobalUniforms : public UniformDataStorage
{
public:
	void SetProjectionMatrix(const Matrix4f& proj);
	Matrix4f GetProjectionMatrix() const { return m_globalVariables.projectionMatrix; }
	void SetVulkanNDCMatrix(const Matrix4f& vndc);
	Matrix4f GetVulkanNDCMatrix() const { return m_globalVariables.vulkanNDC; }
	Matrix4f GetPNMatrix() const { return m_globalVariables.PN; }

	void SetGameWindowSize(const Vector2f& size);
	Vector2f GetGameWindowSize() const { return { m_globalVariables.gameWindowSize.x, m_globalVariables.gameWindowSize.y }; }
	void SetEnvGenWindowSize(const Vector2f& size);
	Vector2f GetEnvGenWindowSize() const { return { m_globalVariables.envGenWindowSize.x, m_globalVariables.envGenWindowSize.y }; }
	void SetShadowGenWindowSize(const Vector2f& size);
	Vector2f GetShadowGenWindowSize() const { return { m_globalVariables.shadowGenWindowSize.x, m_globalVariables.shadowGenWindowSize.y }; }
	void SetSSAOWindowSize(const Vector2f& size);
	Vector2f GetSSAOWindowSize() const { return { m_globalVariables.SSAOWindowSize.x, m_globalVariables.SSAOWindowSize.y }; }
	void SetBloomWindowSize(const Vector2f& size);
	Vector2f GetBloomWindowSize() const { return { m_globalVariables.bloomWindowSize.x, m_globalVariables.bloomWindowSize.y }; }

	void SetMainLightDir(const Vector3f& dir);
	Vector4f GetMainLightDir() const { return m_globalVariables.mainLightDir; }
	void SetMainLightColor(const Vector3f& color);
	Vector4f GetMainLightColor() const { return m_globalVariables.mainLightColor; }
	void SetMainLightVP(const Matrix4f& vp);
	Matrix4f GetmainLightVPN() const { return m_globalVariables.mainLightVPN; }

	void SetRenderSettings(const Vector4f& setting);
	Vector4f GetRenderSettings() const { return m_globalVariables.GEW; }

public:
	bool Init(const std::shared_ptr<GlobalUniforms>& pSelf);
	static std::shared_ptr<GlobalUniforms> Create();

	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void SyncBufferDataInternal() override;
	void SetDirty() override;

	void InitSSAORandomSample();

protected:
	GlobalVariables							m_globalVariables;
};