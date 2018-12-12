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
	Vector4f	SSAOSSRWindowSize;
	Vector4f	bloomWindowSize;
	Vector4f	motionTileWindowSize;	// xy: tile size, zw: window size

	// Scene settings
	Vector4f	mainLightDir;		// Main directional light		w: empty for padding
	Vector4f	mainLightColor;		// Main directional light color	w: empty for padding
	Matrix4f	mainLightVPN;

	// Render settings
	Vector4f	GEW;				// x: Gamma, y: Exposure, z: White Scale, w: empty for padding
	Vector4f	SSRSettings;		// x: BRDFBias, y: SSR mip toggle, rest: reserved
	Vector4f	TemporalSettings0;	// x: Prev motion impact, y: Curr motion impac, z: Max clipped prev ratio, w: Reserved
	Vector4f    TemporalSettings1;	// x: Motion impact lower bound, y: Motion impact upper bound, zw: Reserved

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
	void SetSSAOSSRWindowSize(const Vector2f& size);
	Vector2f GetSSAOSSRWindowSize() const { return { m_globalVariables.SSAOSSRWindowSize.x, m_globalVariables.SSAOSSRWindowSize.y }; }
	void SetBloomWindowSize(const Vector2f& size);
	Vector2f GetBloomWindowSize() const { return { m_globalVariables.bloomWindowSize.x, m_globalVariables.bloomWindowSize.y }; }
	void SetMotionTileSize(const Vector2f& size);
	Vector2f GetMotionTileSize() const { return { m_globalVariables.motionTileWindowSize.x, m_globalVariables.motionTileWindowSize.y }; }
	Vector2f GetMotionTileWindowSize() const { return { m_globalVariables.motionTileWindowSize.z, m_globalVariables.motionTileWindowSize.w }; }

	void SetMainLightDir(const Vector3f& dir);
	Vector4f GetMainLightDir() const { return m_globalVariables.mainLightDir; }
	void SetMainLightColor(const Vector3f& color);
	Vector4f GetMainLightColor() const { return m_globalVariables.mainLightColor; }
	void SetMainLightVP(const Matrix4f& vp);
	Matrix4f GetmainLightVPN() const { return m_globalVariables.mainLightVPN; }

	void SetRenderSettings(const Vector4f& setting);
	Vector4f GetRenderSettings() const { return m_globalVariables.GEW; }
	void SetBRDFBias(float BRDFBias);
	float GetBRDFBias() const { return m_globalVariables.SSRSettings.x; }
	void SetSSRMip(float SSRMip);
	float GetSSRMip() const { return m_globalVariables.SSRSettings.y; }

	void SetTemporalSettings0(const Vector4f& setting);
	Vector4f GetTemporalSettings0() const { return m_globalVariables.TemporalSettings0; }
	void SetTemporalSettings1(const Vector4f& setting);
	Vector4f GetTemporalSettings1() const { return m_globalVariables.TemporalSettings1; }
	void SetPrevMotionImpact(float prevMotionImpact);
	float GetPrevMotionImpact() const { return m_globalVariables.TemporalSettings0.x; }
	void SetCurrMotionImpact(float currMotionImpact);
	float GetCurrMotionImpact() const { return m_globalVariables.TemporalSettings0.y; }
	void SetMaxClippedPrevRatio(float maxClippedPrevRatio);
	float GetMaxClippedPrevRatio() const { return m_globalVariables.TemporalSettings0.z; }
	void SetMotionImpactLowerBound(float motionImpactLowerBound);
	float GetMotionImpactLowerBound() const { return m_globalVariables.TemporalSettings1.x; }
	void SetMotionImpactUpperBound(float motionImpactUpperBound);
	float GetMotionImpactUpperBound() const { return m_globalVariables.TemporalSettings1.y; }

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