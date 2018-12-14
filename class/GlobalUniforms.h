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

	/*******************************************************************
	* DESCRIPTION: Parameters for tone mapping
	*
	* X: Gamma
	* Y: Exposure
	* Z: White scale
	* W: Reserved
	*/
	Vector4f	GEW;

	/*******************************************************************
	* DESCRIPTION: Parameters for stochastic screen space reflection
	*
	* X: BRDFBias
	* Y: SSR mip toggle
	* Z: Sample normal max regen count
	* W: How near reflected vector to surface tangent that normal has to be resampled
	*/
	Vector4f	SSRSettings0;

	/*******************************************************************
	* DESCRIPTION: Parameters for stochastic screen space reflection
	*
	* X: Pixel stride for screen space ray trace
	* Y: Init offset at the beginning of ray trace
	* Z: Max count of ray trace steps
	* W: Thickness of a surface that you can consider it a hit
	*/
	Vector4f	SSRSettings1;

	/*******************************************************************
	* DESCRIPTION: Parameters for stochastic screen space reflection
	*
	* X: How far a hit to the edge of screen that it needs to be fade, to prevent from hard boundary
	* Y: How many steps that a hit starts to fade, to prevent from hard boundary
	* Z: Screen sized mipmap level count
	* W: Reserved
	*/
	Vector4f	SSRSettings2;

	/*******************************************************************
	* DESCRIPTION: Parameters for temporal filter
	*
	* X: Prev motion impact
	* Y: Curr motion impact
	* Z: Max clipped prev ratio
	* W: Reserved
	*/
	Vector4f	TemporalSettings0;	// x: Prev motion impact, y: Curr motion impac, z: Max clipped prev ratio, w: Reserved

	/*******************************************************************
	* DESCRIPTION: Parameters for temporal filter
	*
	* X: Motion impact lower bound
	* Y: Motion impact upper bound
	* Z: Reserved
	* W: Reserved
	*/
	Vector4f    TemporalSettings1;

	/*******************************************************************
	* DESCRIPTION: Parameters for bloom
	*
	* X: Bloom intensity clamping lower bound
	* Y: Bloom intensity clamping upper bound
	* Z: Upsample scale
	* W: Reserved
	*/
	Vector4f    BloomSettings0;

	/*******************************************************************
	* DESCRIPTION: Parameters for bloom
	*
	* X: Bloom amplify when combine back
	* Y: Bloom slope(pow(bloom, slope)), e.g. 1 stands for linear
	* Z: Reserved
	* W: Reserved
	*/
	Vector4f    BloomSettings1;

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

	void SetSSRSettings0(const Vector4f& setting);
	Vector4f GetSSRSettings0() const { return m_globalVariables.SSRSettings0; }
	void SetSSRSettings1(const Vector4f& setting);
	Vector4f GetSSRSettings1() const { return m_globalVariables.SSRSettings1; }
	void SetSSRSettings2(const Vector4f& setting);
	Vector4f GetSSRSettings2() const { return m_globalVariables.SSRSettings2; }
	void SetBRDFBias(float BRDFBias);
	float GetBRDFBias() const { return m_globalVariables.SSRSettings0.x; }
	void SetSSRMip(float SSRMip);
	float GetSSRMip() const { return m_globalVariables.SSRSettings0.y; }
	void SetSampleNormalRegenCount(float count);
	float GetSampleNormalRegenCount() const { return m_globalVariables.SSRSettings0.z; }
	void SetSampleNormalRegenMargin(float margin);
	float GetSampleNormalRegenMargin() const { return m_globalVariables.SSRSettings0.w; }
	void SetSSRTStride(float stride);
	float GetSSRTStride() const { return m_globalVariables.SSRSettings1.x; }
	void SetSSRTInitOffset(float offset);
	float GetSSRTInitOffset() const { return m_globalVariables.SSRSettings1.y; }
	void SetMaxSSRTStepCount(float count);
	float GetMaxSSRTStepCount() const { return m_globalVariables.SSRSettings1.z; }
	void SetSSRTThickness(float thickness);
	float GetSSRTThickness() const { return m_globalVariables.SSRSettings1.w; }
	void SetSSRTBorderFadingDist(float dist);
	float GetSSRTBorderFadingDist() const { return m_globalVariables.SSRSettings2.x; }
	void SetSSRTStepCountFadingDist(float dist);
	float GetSSRTStepCountFadingDist() const { return m_globalVariables.SSRSettings2.y; }
	void SetScreenSizeMipLevel(float mipLevel);
	float GetScreenSizeMipLevel() const { return m_globalVariables.SSRSettings2.z; }

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

	void SetBloomSettings0(const Vector4f& setting);
	Vector4f GetBloomSettings0() const { return m_globalVariables.BloomSettings0; }
	void SetBloomSettings1(const Vector4f& setting);
	Vector4f GetBloomSettings1() const { return m_globalVariables.BloomSettings1; }
	void SetBloomClampingLowerBound(float lowerBound);
	float GetBloomClampingLowerBound() const { return m_globalVariables.BloomSettings0.x; }
	void SetBloomClampingUpperBound(float upperBound);
	float GetBloomClampingUpperBound() const { return m_globalVariables.BloomSettings0.y; }
	void SetUpsampleScale(float upsampleScale);
	float GetUpsampleScale() const { return m_globalVariables.BloomSettings0.z; }
	void SetBloomAmplify(float bloomAmplify);
	float GetBloomAmplify() const { return m_globalVariables.BloomSettings1.x; }
	void SetBloomSlope(float bloomSlope);
	float GetBloomSlope() const { return m_globalVariables.BloomSettings1.y; }

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