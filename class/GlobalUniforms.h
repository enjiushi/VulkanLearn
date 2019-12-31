#pragma once

#include "UniformDataStorage.h"
#include "ChunkBasedUniforms.h"
#include "../Maths/DualQuaternion.h"
#include "../common/Macros.h"
#include <unordered_map>
#include <string>

class DescriptorSetLayout;
class DescriptorSet;
class GlobalTextures;
class Mesh;
class SkeletonAnimationInstance;

const static uint32_t SSAO_SAMPLE_COUNT = 64;
const static uint32_t PLANET_LOD_MAX_LEVEL = 32;

template<typename T>
class GlobalVariables
{
public:

	// Camera settings
	Matrix4x4<T>	projectionMatrix;
	Matrix4x4<T>	prevProjectionMatrix;

	// Windows settings
	Vector4<T>		gameWindowSize;
	Vector4<T>		envGenWindowSize;
	Vector4<T>		shadowGenWindowSize;
	Vector4<T>		SSAOSSRWindowSize;
	Vector4<T>		bloomWindowSize;
	Vector4<T>		motionTileWindowSize;	// xy: tile size, zw: window size

	/*******************************************************************
	* DESCRIPTION: Main directional light direction
	*
	* XYZ: main light direction
	* W: Reserved
	*/
	Vector4<T>		mainLightDir;

	/*******************************************************************
	* DESCRIPTION: Main directional light rgb
	*
	* XYZ: main light color
	* W: Reserved
	*/
	Vector4<T>		mainLightColor;

	/*******************************************************************
	* DESCRIPTION: Main directional light vpn matrix
	*/
	Matrix4x4<T>	mainLightVP;

	/*******************************************************************
	* DESCRIPTION: Camera parameters
	*
	* X: Aspect
	* Y: Film width in meters
	* Z: Film height in meters
	* W: Focal length in meters
	*/
	Vector4<T>	mainCameraSettings0;

	/*******************************************************************
	* DESCRIPTION: Camera parameters
	*
	* X: Focus distance in meters
	* Y: FStop
	* Z: Shutter speed in seconds
	* W: IOS
	*/
	Vector4<T>	mainCameraSettings1;

	/*******************************************************************
	* DESCRIPTION: Camera parameters
	*
	* X: Far plane in meters
	* Y: Horizontal FOV in radius
	* Z: Vertical FOV in radius
	* W: Aperture diameter in meters
	*/
	Vector4<T>	mainCameraSettings2;

	/*******************************************************************
	* DESCRIPTION: Camera parameters
	*
	* X: Tangent(horizontal_fov/2)
	* Y: Tangent(vertical_fov/2)
	* Z: Reserved
	* W: Reserved
	*/
	Vector4<T>	mainCameraSettings3;

	/*******************************************************************
	* DESCRIPTION: Parameters for tone mapping
	*
	* X: Gamma
	* Y: Exposure
	* Z: White scale
	* W: Reserved
	*/
	Vector4<T>	GEW;

	/*******************************************************************
	* DESCRIPTION: Parameters for stochastic screen space reflection
	*
	* X: BRDFBias
	* Y: SSR mip toggle
	* Z: Sample normal max regen count
	* W: How near reflected vector to surface tangent that normal has to be resampled
	*/
	Vector4<T>	SSRSettings0;

	/*******************************************************************
	* DESCRIPTION: Parameters for stochastic screen space reflection
	*
	* X: Pixel stride for screen space ray trace
	* Y: Init offset at the beginning of ray trace
	* Z: Max count of ray trace steps
	* W: Thickness of a surface that you can consider it a hit
	*/
	Vector4<T>	SSRSettings1;

	/*******************************************************************
	* DESCRIPTION: Parameters for stochastic screen space reflection
	*
	* X: How far a hit to the edge of screen that it needs to be fade, to prevent from hard boundary
	* Y: How many steps that a hit starts to fade, to prevent from hard boundary
	* Z: Screen sized mipmap level count
	* W: Reserved
	*/
	Vector4<T>	SSRSettings2;

	/*******************************************************************
	* DESCRIPTION: Parameters for temporal filter
	*
	* X: Motion impact lower bound
	* Y: Motion impact upper bound
	* Z: The portion of high response SSR in a moving region, to slightly reduce ssr noise
	* W: Reserved
	*/
	Vector4<T>  TemporalSettings0;

	/*******************************************************************
	* DESCRIPTION: Parameters for bloom
	*
	* X: Bloom intensity clamping lower bound
	* Y: Bloom intensity clamping upper bound
	* Z: Upsample scale
	* W: Reserved
	*/
	Vector4<T>  BloomSettings0;

	/*******************************************************************
	* DESCRIPTION: Parameters for bloom
	*
	* X: Bloom amplify when combine back
	* Y: Bloom slope(pow(bloom, slope)), e.g. 1 stands for linear
	* Z: Reserved
	* W: Reserved
	*/
	Vector4<T>  BloomSettings1;

	/*******************************************************************
	* DESCRIPTION: Parameters for bloom
	*
	* X: Max size of circle of confusion(COC), measured by texture coordinate(0, 1)
	* Y: Recipe of X
	* Z: COC coefficient: f^2 / (N * (S1 - f) * film_width * 2), divide by 2 means from diameter to radius, divide by film width obtains coc size in texture coordinate(0, 1)
	* W: Reserved
	*/
	Vector4<T>	DOFSettings0;

	/*******************************************************************
	* DESCRIPTION: Parameters for motion blur
	*
	* X: Motion blur amplify factor, to enhance or reduce motion blur effect
	* Y: Motion blur sample count
	* Z: Reserved
	* W: Reserved
	*/
	Vector4<T>	MotionBlurSettings;

	/*******************************************************************
	* DESCRIPTION: Parameters for vignette
	*
	* X: Vignette min distance
	* Y: Vignette max distance
	* Z: Vignette amplify factor
	* W: Reserved
	*/
	Vector4<T>	VignetteSettings;

	/*******************************************************************
	* DESCRIPTION: SSAO Settings
	*
	* X: Sample count
	* Y: SSAO active radius, any ssao factor outside of this radius will fade away
	* Z: SSAO sample length ratio in screen space
	* W: SSAO curve factor -- pow(ssao, curve)
	*/
	Vector4<T>	SSAOSettings;

	/*******************************************************************
	* DESCRIPTION: Planet Rendering Settings
	*
	* X: The ratio in planet radius that transition between rendering raw vertices to normalized spherical vertices
	* Y: Planet triangle screen size
	* Z: Planet max lod level
	* W: Reserved
	*/
	Vector4<T>	PlanetRenderingSettings;

	// Planet LOD level distance look up table
	T			PlanetLODDistanceLUT[PLANET_LOD_MAX_LEVEL];

	// SSAO settings
	Vector4<T>	SSAOSamples[SSAO_SAMPLE_COUNT];
};

typedef GlobalVariables<float> GlobalVariablesf;
typedef GlobalVariables<double> GlobalVariablesd;

class GlobalUniforms : public UniformDataStorage
{
public:
	Vector3d IcosahedronVertices[12];
	uint32_t IcosahedronIndices[60];

public:
	void SetProjectionMatrix(const Matrix4d& proj);
	Matrix4d GetProjectionMatrix() const { return m_globalVariables.projectionMatrix; }

	void SetGameWindowSize(const Vector2d& size);
	Vector2d GetGameWindowSize() const { return { m_globalVariables.gameWindowSize.x, m_globalVariables.gameWindowSize.y }; }
	void SetEnvGenWindowSize(const Vector2d& size);
	Vector2d GetEnvGenWindowSize() const { return { m_globalVariables.envGenWindowSize.x, m_globalVariables.envGenWindowSize.y }; }
	void SetShadowGenWindowSize(const Vector2d& size);
	Vector2d GetShadowGenWindowSize() const { return { m_globalVariables.shadowGenWindowSize.x, m_globalVariables.shadowGenWindowSize.y }; }
	void SetSSAOSSRWindowSize(const Vector2d& size);
	Vector2d GetSSAOSSRWindowSize() const { return { m_globalVariables.SSAOSSRWindowSize.x, m_globalVariables.SSAOSSRWindowSize.y }; }
	void SetBloomWindowSize(const Vector2d& size);
	Vector2d GetBloomWindowSize() const { return { m_globalVariables.bloomWindowSize.x, m_globalVariables.bloomWindowSize.y }; }
	void SetMotionTileSize(const Vector2d& size);
	Vector2d GetMotionTileSize() const { return { m_globalVariables.motionTileWindowSize.x, m_globalVariables.motionTileWindowSize.y }; }
	Vector2d GetMotionTileWindowSize() const { return { m_globalVariables.motionTileWindowSize.z, m_globalVariables.motionTileWindowSize.w }; }

	void SetMainLightDir(const Vector3d& dir);
	Vector4d GetMainLightDir() const { return m_globalVariables.mainLightDir; }
	void SetMainLightColor(const Vector3d& color);
	Vector4d GetMainLightColor() const { return m_globalVariables.mainLightColor; }
	void SetMainLightVP(const Matrix4d& vp);
	Matrix4d GetmainLightVP() const { return m_globalVariables.mainLightVP; }

	void SetMainCameraSettings0(const Vector4d& settings);
	Vector4d GetMainCameraSettings0() const { return m_globalVariables.mainCameraSettings0; }
	void SetMainCameraSettings1(const Vector4d& settings);
	Vector4d GetMainCameraSettings1() const { return m_globalVariables.mainCameraSettings1; }
	void SetMainCameraSettings2(const Vector4d& settings);
	Vector4d GetMainCameraSettings2() const { return m_globalVariables.mainCameraSettings2; }
	void SetMainCameraSettings3(const Vector4d& settings);
	Vector4d GetMainCameraSettings3() const { return m_globalVariables.mainCameraSettings3; }
	void SetMainCameraAspect(double aspect);
	double GetMainCameraAspect() const { return m_globalVariables.mainCameraSettings0.x; }
	void SetMainCameraFilmWidth(double filmWidth);
	double GetMainCameraFilmWidth() const { return m_globalVariables.mainCameraSettings0.y; }
	void SetMainCameraFilmHeight(double filmHeight);
	double GetMainCameraFilmHeight() const { return m_globalVariables.mainCameraSettings0.z; }
	void SetMainCameraFocalLength(double focalLength);
	double GetMainCameraFocalLength() const { return m_globalVariables.mainCameraSettings0.w; }
	void SetMainCameraFocusDistance(double focusDistance);
	double GetMainCameraFocusDistance() const { return m_globalVariables.mainCameraSettings1.x; }
	void SetMainCameraFStop(double fstop);
	double GetMainCameraFStop() const { return m_globalVariables.mainCameraSettings1.y; }
	void SetMainCameraShutterSpeed(double shutterSpeed);
	double GetMainCameraShutterSpeed() const { return m_globalVariables.mainCameraSettings1.z; }
	void SetMainCameraISO(double ISO);
	double GetMainCameraISO() const { return m_globalVariables.mainCameraSettings1.w; }
	void SetMainCameraFarPlane(double farPlane);
	double GetMainCameraFarPlane() const { return m_globalVariables.mainCameraSettings2.x; }
	void SetMainCameraHorizontalFOV(double horizontalFOV);
	double GetMainCameraHorizontalFOV() const { return m_globalVariables.mainCameraSettings2.y; }
	void SetMainCameraVerticalFOV(double verticalFOV);
	double GetMainCameraVerticalFOV() const { return m_globalVariables.mainCameraSettings2.z; }
	void SetMainCameraApertureDiameter(double apertureDiameter);
	double GetMainCameraApertureDiameter() const { return m_globalVariables.mainCameraSettings2.w; }
	void SetMainCameraHorizontalTangentFOV_2(double tangentHorizontalFOV_2);
	double GetMainCameraHorizontalTangentFOV_2() const { return m_globalVariables.mainCameraSettings3.x; }
	void SetMainCameraVerticalTangentFOV_2(double tangentVerticalFOV_2);
	double GetMainCameraVerticalTangentFOV_2() const { return m_globalVariables.mainCameraSettings3.y; }

	void SetRenderSettings(const Vector4d& setting);
	Vector4d GetRenderSettings() const { return m_globalVariables.GEW; }

	void SetSSRSettings0(const Vector4d& setting);
	Vector4d GetSSRSettings0() const { return m_globalVariables.SSRSettings0; }
	void SetSSRSettings1(const Vector4d& setting);
	Vector4d GetSSRSettings1() const { return m_globalVariables.SSRSettings1; }
	void SetSSRSettings2(const Vector4d& setting);
	Vector4d GetSSRSettings2() const { return m_globalVariables.SSRSettings2; }
	void SetBRDFBias(double BRDFBias);
	double GetBRDFBias() const { return m_globalVariables.SSRSettings0.x; }
	void SetSSRMip(double SSRMip);
	double GetSSRMip() const { return m_globalVariables.SSRSettings0.y; }
	void SetSampleNormalRegenCount(double count);
	double GetSampleNormalRegenCount() const { return m_globalVariables.SSRSettings0.z; }
	void SetSampleNormalRegenMargin(double margin);
	double GetSampleNormalRegenMargin() const { return m_globalVariables.SSRSettings0.w; }
	void SetSSRTStride(double stride);
	double GetSSRTStride() const { return m_globalVariables.SSRSettings1.x; }
	void SetSSRTInitOffset(double offset);
	double GetSSRTInitOffset() const { return m_globalVariables.SSRSettings1.y; }
	void SetMaxSSRTStepCount(double count);
	double GetMaxSSRTStepCount() const { return m_globalVariables.SSRSettings1.z; }
	void SetSSRTThickness(double thickness);
	double GetSSRTThickness() const { return m_globalVariables.SSRSettings1.w; }
	void SetSSRTBorderFadingDist(double dist);
	double GetSSRTBorderFadingDist() const { return m_globalVariables.SSRSettings2.x; }
	void SetSSRTStepCountFadingDist(double dist);
	double GetSSRTStepCountFadingDist() const { return m_globalVariables.SSRSettings2.y; }
	void SetScreenSizeMipLevel(double mipLevel);
	double GetScreenSizeMipLevel() const { return m_globalVariables.SSRSettings2.z; }

	void SetTemporalSettings0(const Vector4d& setting);
	Vector4d GetTemporalSettings0() const { return m_globalVariables.TemporalSettings0; }
	void SetMotionImpactLowerBound(double motionImpactLowerBound);
	double GetMotionImpactLowerBound() const { return m_globalVariables.TemporalSettings0.x; }
	void SetMotionImpactUpperBound(double motionImpactUpperBound);
	double GetMotionImpactUpperBound() const { return m_globalVariables.TemporalSettings0.y; }
	void SetHighResponseSSRPortion(double highResponseSSRPortion);
	double GetHighResponseSSRPortion() const { return m_globalVariables.TemporalSettings0.z; }

	void SetBloomSettings0(const Vector4d& setting);
	Vector4d GetBloomSettings0() const { return m_globalVariables.BloomSettings0; }
	void SetBloomSettings1(const Vector4d& setting);
	Vector4d GetBloomSettings1() const { return m_globalVariables.BloomSettings1; }
	void SetBloomClampingLowerBound(double lowerBound);
	double GetBloomClampingLowerBound() const { return m_globalVariables.BloomSettings0.x; }
	void SetBloomClampingUpperBound(double upperBound);
	double GetBloomClampingUpperBound() const { return m_globalVariables.BloomSettings0.y; }
	void SetUpsampleScale(double upsampleScale);
	double GetUpsampleScale() const { return m_globalVariables.BloomSettings0.z; }
	void SetBloomAmplify(double bloomAmplify);
	double GetBloomAmplify() const { return m_globalVariables.BloomSettings1.x; }
	void SetBloomSlope(double bloomSlope);
	double GetBloomSlope() const { return m_globalVariables.BloomSettings1.y; }

	void SetMaxCOC(double maxCOC);
	double GetMaxCOC() const { return m_globalVariables.DOFSettings0.x; }
	double GetRcpMaxCOC() const { return m_globalVariables.DOFSettings0.y; }
	double GetCOCCoeff() const { return m_globalVariables.DOFSettings0.z; }

	void SetMotionBlurSettings(const Vector4d& settings);
	void SetMotionBlurAmplify(double motionBlurAmplify);
	double GetMotionBlurAmplify() const { return m_globalVariables.MotionBlurSettings.x; }
	void SetMotionBlurSampleCount(uint32_t sampleCount);
	uint32_t GetMotionBlurSampleCount() const { return (uint32_t)m_globalVariables.MotionBlurSettings.y; }

	void SetVignetteSettings(const Vector4d& settings);
	void SetVignetteMinDist(double minDist);
	double GetVignetteMinDist() const { return m_globalVariables.VignetteSettings.x; }
	void SetVignetteMaxDist(double maxDist);
	double GetVignetteMaxDist() const { return m_globalVariables.VignetteSettings.y; }
	void SetVignetteAmplify(double vignetteAmplify);
	double GetVignetteAmplify() const { return m_globalVariables.VignetteSettings.z; }

	void SetSSAOSampleCount(double sampleCount);
	double GetSSAOSampleCount() const { return m_globalVariables.SSAOSettings.x; }
	void SetSSAOSampleRadius(double radius);
	double GetSSAOSampleRadius() const { return m_globalVariables.SSAOSettings.y; }
	void SetSSAOScreenSpaceSampleLength(double length);
	double GetSSAOScreenSpaceSampleLength() const { return m_globalVariables.SSAOSettings.z; }
	void SetSSAOCurveFactor(double factor);
	double GetSSAOCurveFactor() const { return m_globalVariables.SSAOSettings.w; }

	void SetPlanetSphericalTransitionRatio(double ratio);
	double GetPlanetSphericalTransitionRatio() const { return m_globalVariables.PlanetRenderingSettings.x; }
	void SetPlanetTriangleScreenSize(double size);
	double GetPlanetTriangleScreenSize() const { return m_globalVariables.PlanetRenderingSettings.y; }
	void SetMaxPlanetLODLevel(double maxLevel);
	double GetMaxPlanetLODLevel() const { return m_globalVariables.PlanetRenderingSettings.z; }

	double GetLODDistance(uint32_t level) const { return m_globalVariables.PlanetLODDistanceLUT[level]; }

public:
	bool Init(const std::shared_ptr<GlobalUniforms>& pSelf);
	static std::shared_ptr<GlobalUniforms> Create();

	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void UpdateUniformDataInternal() override;
	void SetDirtyInternal() override;
	const void* AcquireDataPtr() const override { return &m_singlePrecisionGlobalVariables; }
	uint32_t AcquireDataSize() const override { return sizeof(GlobalVariables<float>); }

	void InitSSAORandomSample();
	void InitIcosahedron();
	void InitPlanetLODDistanceLUT();

protected:
	GlobalVariablesd	m_globalVariables;
	GlobalVariablesf	m_singlePrecisionGlobalVariables;
};

template <typename T>
class BoneData
{
public:
	DualQuaternion<T>	currBoneOffsetDQ;
	DualQuaternion<T>	prevBoneOffsetDQ;
};

class BoneIndirectUniform;

class PerBoneUniforms : public ChunkBasedUniforms
{
protected:
	bool Init(const std::shared_ptr<PerBoneUniforms>& pSelf);

public:
	static std::shared_ptr<PerBoneUniforms> Create();

public:
	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void SetBoneOffsetTransform(uint32_t chunkIndex, const DualQuaterniond& offsetDQ);
	DualQuaterniond GetBoneOffsetTransform(uint32_t chunkIndex) const;

protected:
	void UpdateDirtyChunkInternal(uint32_t index) override;
	const void* AcquireDataPtr() const override { return &m_singlePrecisionBoneData[0]; }
	uint32_t AcquireDataSize() const override { return sizeof(m_singlePrecisionBoneData); }

protected:
	BoneData<double>	m_boneData[MAXIMUM_OBJECTS];
	BoneData<float>		m_singlePrecisionBoneData[MAXIMUM_OBJECTS];

	friend class BoneIndirectUniform;
};

class Mesh;
class SkeletonAnimationInstance;
class AnimationController;

class BoneIndirectUniform : public ChunkBasedUniforms
{
	typedef struct BoneIndirectInfo
	{
		std::wstring	boneName;
		uint32_t		boneIndex;
	}BoneIndirectInfo;

public:
	typedef std::unordered_map<std::size_t, BoneIndirectInfo>	BoneIndexLookupTable;

protected:
	bool Init(const std::shared_ptr<BoneIndirectUniform>& pSelf, uint32_t type);

public:
	static std::shared_ptr<BoneIndirectUniform> Create(uint32_t type);
	uint32_t AllocatePerObjectChunk() override { ASSERTION(false); return -1; }
	uint32_t AllocateConsecutiveChunks(uint32_t chunkSize) override;

	bool GetBoneIndex(uint32_t chunkIndex, std::size_t hashCode, BoneIndexLookupTable::iterator& it);
	bool GetBoneCount(uint32_t chunkIndex, uint32_t& outBoneCount) const;

	std::size_t GetBoneHashCode(uint32_t chunkIndex, uint32_t index) const;

	// Disable the visibility of these access functions, since meshChunkIndex is something internal only within mesh
	// Let specific mesh to deal with these functions and make wrappers of them
protected:
	// Bone index automatically generated if not exists
	void SetBoneTransform(uint32_t chunkIndex, std::size_t hashCode, const DualQuaterniond& offsetDQ);
	bool GetBoneInfo(uint32_t chunkIndex, std::size_t hashCode, uint32_t& outBoneIndex, DualQuaterniond& outBoneOffsetTransformDQ);

	// Input bone index
	void SetBoneTransform(uint32_t chunkIndex, std::size_t hashCode, uint32_t boneIndex, const DualQuaterniond& offsetDQ);
	bool GetBoneTransform(uint32_t chunkIndex, std::size_t hashCode, uint32_t boneIndex, DualQuaterniond& outBoneOffsetTransformDQ) const;

public:
	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void UpdateDirtyChunkInternal(uint32_t index) override;
	const void* AcquireDataPtr() const override { return &m_boneChunkIndex[0]; }
	uint32_t AcquireDataSize() const override { return sizeof(m_boneChunkIndex); }

protected:
	uint32_t									m_boneChunkIndex[MAXIMUM_OBJECTS];
	// index stands for instance chunk index of a set of bones
	std::unordered_map<uint32_t, BoneIndexLookupTable>	m_boneIndexLookupTables;

	uint32_t									m_boneBufferType;			// Which binding bone data buffer is located

	friend class Mesh;
	friend class SkeletonAnimationInstance;
	friend class AnimationController;
};

class PerMeshUniforms : public ChunkBasedUniforms
{
	typedef struct _MeshData
	{
		uint32_t	boneChunkIndexOffset;
	}MeshData;

protected:
	bool Init(const std::shared_ptr<PerMeshUniforms>& pSelf);

public:
	static std::shared_ptr<PerMeshUniforms> Create();

public:
	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void SetBoneChunkIndexOffset(uint32_t chunkIndex, uint32_t boneChunkIndexOffset);
	uint32_t GetBoneChunkIndexOffset(uint32_t chunkIndex) const { return m_meshData[chunkIndex].boneChunkIndexOffset; }

protected:
	void UpdateDirtyChunkInternal(uint32_t index) override;
	const void* AcquireDataPtr() const override { return &m_meshData[0]; }
	uint32_t AcquireDataSize() const override { return sizeof(m_meshData); }

protected:
	MeshData	m_meshData[MAXIMUM_OBJECTS];

	friend class Mesh;
};

class PerAnimationUniforms : public ChunkBasedUniforms
{
	typedef struct _AnimationData
	{
		uint32_t	boneChunkIndexOffset;
	}AnimationData;

protected:
	bool Init(const std::shared_ptr<PerAnimationUniforms>& pSelf);

public:
	static std::shared_ptr<PerAnimationUniforms> Create();

public:
	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void SetBoneChunkIndexOffset(uint32_t chunkIndex, uint32_t boneChunkIndexOffset);
	uint32_t GetBoneChunkIndexOffset(uint32_t chunkIndex) const { return m_animationData[chunkIndex].boneChunkIndexOffset; }

protected:
	void UpdateDirtyChunkInternal(uint32_t index) override;
	const void* AcquireDataPtr() const override { return &m_animationData[0]; }
	uint32_t AcquireDataSize() const override { return sizeof(m_animationData); }

protected:
	AnimationData	m_animationData[MAXIMUM_OBJECTS];

	friend class SkeletonAnimationInstance;
};