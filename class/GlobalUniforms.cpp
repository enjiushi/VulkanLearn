#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/Image.h"
#include "../vulkan/Texture2D.h"
#include "../vulkan/Texture2DArray.h"
#include "../vulkan/TextureCube.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "UniformData.h"
#include "GlobalTextures.h"
#include "GlobalUniforms.h"
#include "Material.h"
#include <random>

bool GlobalUniforms::Init(const std::shared_ptr<GlobalUniforms>& pSelf)
{
	if (!UniformDataStorage::Init(pSelf, sizeof(m_singlePrecisionGlobalVariables), PerFrameDataStorage::Uniform))
		return false;

	SetGameWindowSize({ (double)GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, (double)GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height });
	SetEnvGenWindowSize({ (double)FrameBufferDiction::ENV_GEN_WINDOW_SIZE, (double)FrameBufferDiction::ENV_GEN_WINDOW_SIZE });
	SetShadowGenWindowSize({ (double)FrameBufferDiction::SHADOW_GEN_WINDOW_SIZE, (double)FrameBufferDiction::SHADOW_GEN_WINDOW_SIZE });
	SetSSAOSSRWindowSize({ (double)FrameBufferDiction::SSAO_SSR_WINDOW_WIDTH, (double)FrameBufferDiction::SSAO_SSR_WINDOW_HEIGHT });
	SetBloomWindowSize({ (double)FrameBufferDiction::BLOOM_WINDOW_SIZE, (double)FrameBufferDiction::BLOOM_WINDOW_SIZE });
	SetMotionTileSize({ (double)FrameBufferDiction::MOTION_TILE_SIZE, (double)FrameBufferDiction::MOTION_TILE_SIZE });

	InitSSAORandomSample();

	return true;
}

std::shared_ptr<GlobalUniforms> GlobalUniforms::Create()
{
	std::shared_ptr<GlobalUniforms> pGlobalUniforms = std::make_shared<GlobalUniforms>();
	if (pGlobalUniforms.get() && pGlobalUniforms->Init(pGlobalUniforms))
		return pGlobalUniforms;
	return nullptr;
}

void GlobalUniforms::SetProjectionMatrix(const Matrix4d& proj) 
{ 
	// Since prev proj matrix is used to calculate per-pixel vocelity buffer,
	// we need to handle the impact of camera jitter for temporal.
	// Using the same proj matrix for both prev and curr, which behaves exactly what we want here
	m_globalVariables.prevProjectionMatrix = proj;
	m_globalVariables.projectionMatrix = proj;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, prevProjectionMatrix);
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, projectionMatrix);
	SetDirty();
}

void GlobalUniforms::SetGameWindowSize(const Vector2d& size)
{
	m_globalVariables.gameWindowSize.x = size.x;
	m_globalVariables.gameWindowSize.y = size.y;
	m_globalVariables.gameWindowSize.z = 1.0 / size.x;
	m_globalVariables.gameWindowSize.w = 1.0 / size.y;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, gameWindowSize);
	SetDirty();
}

void GlobalUniforms::SetEnvGenWindowSize(const Vector2d& size)
{
	m_globalVariables.envGenWindowSize.x = size.x;
	m_globalVariables.envGenWindowSize.y = size.y;
	m_globalVariables.envGenWindowSize.z = 1.0 / size.x;
	m_globalVariables.envGenWindowSize.w = 1.0 / size.y;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, envGenWindowSize);
	SetDirty();
}

void GlobalUniforms::SetShadowGenWindowSize(const Vector2d& size)
{
	m_globalVariables.shadowGenWindowSize.x = size.x;
	m_globalVariables.shadowGenWindowSize.y = size.y;
	m_globalVariables.shadowGenWindowSize.z = 1.0 / size.x;
	m_globalVariables.shadowGenWindowSize.w = 1.0 / size.y;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, shadowGenWindowSize);
	SetDirty();
}


void GlobalUniforms::SetSSAOSSRWindowSize(const Vector2d& size)
{
	m_globalVariables.SSAOSSRWindowSize.x = size.x;
	m_globalVariables.SSAOSSRWindowSize.y = size.y;
	m_globalVariables.SSAOSSRWindowSize.z = 1.0 / size.x;
	m_globalVariables.SSAOSSRWindowSize.w = 1.0 / size.y;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, SSAOSSRWindowSize);
	SetDirty();
}

void GlobalUniforms::SetBloomWindowSize(const Vector2d& size)
{
	m_globalVariables.bloomWindowSize.x = size.x;
	m_globalVariables.bloomWindowSize.y = size.y;
	m_globalVariables.bloomWindowSize.z = 1.0 / size.x;
	m_globalVariables.bloomWindowSize.w = 1.0 / size.y;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, bloomWindowSize);
	SetDirty();
}

void GlobalUniforms::SetMotionTileSize(const Vector2d& size)
{
	m_globalVariables.motionTileWindowSize.x = size.x;
	m_globalVariables.motionTileWindowSize.y = size.y;
	m_globalVariables.motionTileWindowSize.z = (uint32_t)m_globalVariables.gameWindowSize.x / (uint32_t)size.x + (((uint32_t)m_globalVariables.gameWindowSize.x % (uint32_t)size.x) > 0 ? 1 : 0);
	m_globalVariables.motionTileWindowSize.w = (uint32_t)m_globalVariables.gameWindowSize.y / (uint32_t)size.y + (((uint32_t)m_globalVariables.gameWindowSize.y % (uint32_t)size.y) > 0 ? 1 : 0);
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, motionTileWindowSize);
	SetDirty();
}

void GlobalUniforms::UpdateUniformDataInternal()
{
	m_globalVariables.DOFSettings0.z = m_globalVariables.mainCameraSettings0.w * m_globalVariables.mainCameraSettings0.w / 
		(m_globalVariables.mainCameraSettings1.y * (m_globalVariables.mainCameraSettings1.x - m_globalVariables.mainCameraSettings0.w) * m_globalVariables.mainCameraSettings0.y * 2.0f);
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, DOFSettings0.z);
}

void GlobalUniforms::SetDirtyInternal()
{
}

void GlobalUniforms::SetMainLightDir(const Vector3d& dir)
{
	m_globalVariables.mainLightDir = dir.Normal();
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, mainLightDir);
	SetDirty();
}

void GlobalUniforms::SetMainLightColor(const Vector3d& color)
{
	m_globalVariables.mainLightColor = color;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, mainLightColor);
	SetDirty();
}

void GlobalUniforms::SetMainLightVP(const Matrix4d& vp)
{
	m_globalVariables.mainLightVP = vp;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, mainLightVP);
	SetDirty();
}

void GlobalUniforms::SetMainCameraSettings0(const Vector4d& settings)
{
	m_globalVariables.mainCameraSettings0 = settings;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings0);
	SetDirty();
}

void GlobalUniforms::SetMainCameraSettings1(const Vector4d& settings)
{
	m_globalVariables.mainCameraSettings1 = settings;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings1);
	SetDirty();
}

void GlobalUniforms::SetMainCameraSettings2(const Vector4d& settings)
{
	m_globalVariables.mainCameraSettings2 = settings;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings2);
	SetDirty();
}

void GlobalUniforms::SetMainCameraSettings3(const Vector4d& settings)
{
	m_globalVariables.mainCameraSettings3 = settings;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings3);
	SetDirty();
}

void GlobalUniforms::SetMainCameraAspect(double aspect)
{
	m_globalVariables.mainCameraSettings0.x = aspect;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings0.x);
	SetDirty();
}

void GlobalUniforms::SetMainCameraFilmWidth(double filmWidth)
{
	m_globalVariables.mainCameraSettings0.y = filmWidth;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings0.y);
	SetDirty();
}

// start from here
void GlobalUniforms::SetMainCameraFilmHeight(double filmHeight)
{
	m_globalVariables.mainCameraSettings0.z = filmHeight;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings0.z);
	SetDirty();
}

void GlobalUniforms::SetMainCameraFocalLength(double focalLength)
{
	m_globalVariables.mainCameraSettings0.w = focalLength;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings0.w);
	SetDirty();
}

void GlobalUniforms::SetMainCameraFocusDistance(double focusDistance)
{
	m_globalVariables.mainCameraSettings1.x = focusDistance;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings1.x);
	SetDirty();
}

void GlobalUniforms::SetMainCameraFStop(double fstop)
{
	m_globalVariables.mainCameraSettings1.y = fstop;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings1.y);
	SetDirty();
}

void GlobalUniforms::SetMainCameraShutterSpeed(double shutterSpeed)
{
	m_globalVariables.mainCameraSettings1.z = shutterSpeed;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings1.z);
	SetDirty();
}

void GlobalUniforms::SetMainCameraISO(double ISO)
{
	m_globalVariables.mainCameraSettings1.w = ISO;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings1.w);
	SetDirty();
}

void GlobalUniforms::SetMainCameraFarPlane(double farPlane)
{
	m_globalVariables.mainCameraSettings2.x = farPlane;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings2.x);
	SetDirty();
}

void GlobalUniforms::SetMainCameraHorizontalFOV(double horizontalFOV)
{
	m_globalVariables.mainCameraSettings2.y = horizontalFOV;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings2.y);
	SetDirty();
}

void GlobalUniforms::SetMainCameraVerticalFOV(double verticalFOV)
{
	m_globalVariables.mainCameraSettings2.z = verticalFOV;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings2.z);
	SetDirty();
}

void GlobalUniforms::SetMainCameraApertureDiameter(double apertureDiameter)
{
	m_globalVariables.mainCameraSettings2.w = apertureDiameter;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings2.w);
	SetDirty();
}

void GlobalUniforms::SetMainCameraHorizontalTangentFOV_2(double tangentHorizontalFOV_2)
{
	m_globalVariables.mainCameraSettings3.x = tangentHorizontalFOV_2;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings3.x);
	SetDirty();
}

void GlobalUniforms::SetMainCameraVerticalTangentFOV_2(double tangentVerticalFOV_2)
{
	m_globalVariables.mainCameraSettings3.y = tangentVerticalFOV_2;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, mainCameraSettings3.y);
	SetDirty();
}

void GlobalUniforms::SetRenderSettings(const Vector4d& setting)
{
	m_globalVariables.GEW = setting;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, GEW);
	SetDirty();
}

void GlobalUniforms::SetSSRSettings0(const Vector4d& setting)
{
	m_globalVariables.SSRSettings0 = setting;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings0);
	SetDirty();
}

void GlobalUniforms::SetSSRSettings1(const Vector4d& setting)
{
	m_globalVariables.SSRSettings1 = setting;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings1);
	SetDirty();
}

void GlobalUniforms::SetSSRSettings2(const Vector4d& setting)
{
	m_globalVariables.SSRSettings2 = setting;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings2);
	SetDirty();
}

void GlobalUniforms::SetBRDFBias(double BRDFBias)
{
	m_globalVariables.SSRSettings0.x = BRDFBias;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings0.x);
	SetDirty();
}

void GlobalUniforms::SetSSRMip(double SSRMip)
{
	m_globalVariables.SSRSettings0.y = SSRMip;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings0.y);
	SetDirty();
}

void GlobalUniforms::SetSampleNormalRegenCount(double count)
{
	m_globalVariables.SSRSettings0.z = count;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings0.z);
	SetDirty();
}

void GlobalUniforms::SetSampleNormalRegenMargin(double margin)
{
	m_globalVariables.SSRSettings0.w = margin;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings0.w);
	SetDirty();
}

void GlobalUniforms::SetSSRTStride(double stride)
{
	m_globalVariables.SSRSettings1.x = stride;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings1.x);
	SetDirty();
}

void GlobalUniforms::SetSSRTInitOffset(double offset)
{
	m_globalVariables.SSRSettings1.y = offset;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings1.y);
	SetDirty();
}

void GlobalUniforms::SetMaxSSRTStepCount(double count)
{
	m_globalVariables.SSRSettings1.z = count;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings1.z);
	SetDirty();
}

void GlobalUniforms::SetSSRTThickness(double thickness)
{
	m_globalVariables.SSRSettings1.w = thickness;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings1.w);
	SetDirty();
}

void GlobalUniforms::SetSSRTBorderFadingDist(double dist)
{
	m_globalVariables.SSRSettings2.x = dist;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings2.x);
	SetDirty();
}

void GlobalUniforms::SetSSRTStepCountFadingDist(double dist)
{
	m_globalVariables.SSRSettings2.y = dist;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings2.y);
	SetDirty();
}

void GlobalUniforms::SetScreenSizeMipLevel(double mipLevel)
{
	m_globalVariables.SSRSettings2.z = mipLevel;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSRSettings2.z);
	SetDirty();
}

void GlobalUniforms::SetTemporalSettings0(const Vector4d& setting)
{
	m_globalVariables.TemporalSettings0 = setting;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, TemporalSettings0);
	SetDirty();
}

void GlobalUniforms::SetMotionImpactLowerBound(double motionImpactLowerBound)
{
	m_globalVariables.TemporalSettings0.x = motionImpactLowerBound;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, TemporalSettings0.x);
	SetDirty();
}

void GlobalUniforms::SetMotionImpactUpperBound(double motionImpactUpperBound)
{
	m_globalVariables.TemporalSettings0.y = motionImpactUpperBound;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, TemporalSettings0.y);
	SetDirty();
}

void GlobalUniforms::SetHighResponseSSRPortion(double highResponseSSRPortion)
{
	m_globalVariables.TemporalSettings0.z = highResponseSSRPortion;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, TemporalSettings0.z);
	SetDirty();
}

void GlobalUniforms::SetBloomSettings0(const Vector4d& setting)
{
	m_globalVariables.BloomSettings0 = setting;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, BloomSettings0);
	SetDirty();
}

void GlobalUniforms::SetBloomSettings1(const Vector4d& setting)
{
	m_globalVariables.BloomSettings1 = setting;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, BloomSettings1);
	SetDirty();
}

void GlobalUniforms::SetBloomClampingLowerBound(double lowerBound)
{
	m_globalVariables.BloomSettings0.x = lowerBound;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, BloomSettings0.x);
	SetDirty();
}

void GlobalUniforms::SetBloomClampingUpperBound(double upperBound)
{
	m_globalVariables.BloomSettings0.y = upperBound;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, BloomSettings0.y);
	SetDirty();
}

void GlobalUniforms::SetUpsampleScale(double upsampleScale)
{
	m_globalVariables.BloomSettings0.z = upsampleScale;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, BloomSettings0.z);
	SetDirty();
}

void GlobalUniforms::SetBloomAmplify(double bloomAmplify)
{
	m_globalVariables.BloomSettings1.x = bloomAmplify;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, BloomSettings1.x);
	SetDirty();
}

void GlobalUniforms::SetBloomSlope(double bloomSlope)
{
	m_globalVariables.BloomSettings1.y = bloomSlope;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, BloomSettings1.y);
	SetDirty();
}

void GlobalUniforms::SetMaxCOC(double maxCOC)
{
	m_globalVariables.DOFSettings0.x = maxCOC;
	m_globalVariables.DOFSettings0.y = 1 / maxCOC;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, DOFSettings0.x);
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, DOFSettings0.y);
	SetDirty();
}

void GlobalUniforms::SetMotionBlurSettings(const Vector4d& settings)
{
	m_globalVariables.MotionBlurSettings = settings;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, MotionBlurSettings);
	SetDirty();
}

void GlobalUniforms::SetMotionBlurAmplify(double motionBlurAmplify)
{
	m_globalVariables.MotionBlurSettings.x = motionBlurAmplify;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, MotionBlurSettings.x);
	SetDirty();
}

void GlobalUniforms::SetMotionBlurSampleCount(uint32_t sampleCount)
{
	m_globalVariables.MotionBlurSettings.y = (double)sampleCount;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, MotionBlurSettings.y);
	SetDirty();
}

void GlobalUniforms::SetVignetteSettings(const Vector4d& settings)
{
	m_globalVariables.VignetteSettings = settings;
	CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, VignetteSettings);
	SetDirty();
}

void GlobalUniforms::SetVignetteMinDist(double minDist)
{
	m_globalVariables.VignetteSettings.x = minDist;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, VignetteSettings.x);
	SetDirty();
}

void GlobalUniforms::SetVignetteMaxDist(double maxDist)
{
	m_globalVariables.VignetteSettings.y = maxDist;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, VignetteSettings.y);
	SetDirty();
}

void GlobalUniforms::SetVignetteAmplify(double vignetteAmplify)
{
	m_globalVariables.VignetteSettings.z = vignetteAmplify;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, VignetteSettings.z);
	SetDirty();
}

void GlobalUniforms::SetSSAOSampleCount(double sampleCount)
{
	m_globalVariables.SSAOSettings.x = sampleCount;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSAOSettings.x);
	SetDirty();
}

void GlobalUniforms::SetSSAOSampleRadius(double radius)
{
	m_globalVariables.SSAOSettings.y = radius;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSAOSettings.y);
	SetDirty();
}

void GlobalUniforms::SetSSAOScreenSpaceSampleLength(double length)
{
	m_globalVariables.SSAOSettings.z = length;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSAOSettings.z);
	SetDirty();
}

void GlobalUniforms::SetSSAOCurveFactor(double factor)
{
	m_globalVariables.SSAOSettings.w = factor;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, SSAOSettings.w);
	SetDirty();
}

void GlobalUniforms::SetPlanetSphericalTransitionRatio(double ratio)
{
	m_globalVariables.PlanetRenderingSettings.x = ratio;
	CONVERT2SINGLEVAL(m_globalVariables, m_singlePrecisionGlobalVariables, PlanetRenderingSettings.x);
	SetDirty();
}




std::vector<UniformVarList> GlobalUniforms::PrepareUniformVarList() const
{
	std::vector<UniformVarList> list = 
	{
		{
			DynamicUniformBuffer,
			"GlobalUniforms",
			{
				{
					Mat4Unit,
					"ProjectionMatrix"
				},
				{
					Mat4Unit,
					"PrevProjectionMatrix"
				},
				{
					Vec4Unit,
					"GameWindowSize"
				},
				{
					Vec4Unit,
					"EnvGenWindowSize"
				},
				{
					Vec4Unit,
					"ShadowGenWindowSize"
				},
				{
					Vec4Unit,
					"SSAOWindowSize"
				},
				{
					Vec4Unit,
					"BloomWindowSize"
				},
				{
					Vec4Unit,
					"MotionWindowSize"
				},
				{
					Vec4Unit,
					"MainLightDir"
				},
				{
					Vec4Unit,
					"MainLightColor"
				},
				{
					Mat4Unit,
					"MainLightVP"
				},
				{
					Vec4Unit,
					"Main Camera settings0"
				},
				{
					Vec4Unit,
					"Main Camera settings1"
				},
				{
					Vec4Unit,
					"Main Camera settings2"
				},
				{
					Vec4Unit,
					"Main Camera settings3"
				},
				{
					Vec4Unit,
					"Settings: Gamma, Exposure, White Scale"
				},
				{
					Vec4Unit,
					"SSR Settings0"
				},
				{
					Vec4Unit,
					"SSR Settings1"
				},
				{
					Vec4Unit,
					"SSR Settings2"
				},
				{
					Vec4Unit,
					"Temporal Settings0"
				},
				{
					Vec4Unit,
					"Bloom Settings0"
				},
				{
					Vec4Unit,
					"Bloom Settings1"
				},
				{
					Vec4Unit,
					"Depth of field Settings0"
				},
				{
					Vec4Unit,
					"Motion blur settings"
				},
				{
					Vec4Unit,
					"Vignette settings"
				},
				{
					Vec4Unit,
					"SSAO settings"
				},
				{
					Vec4Unit,
					"SSAO Samples",
					SSAO_SAMPLE_COUNT
				}
			}
		}
	};

	return list;
}

uint32_t GlobalUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateUniformBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<UniformBuffer>(GetBuffer()));

	return bindingIndex;
}

void GlobalUniforms::InitSSAORandomSample()
{
	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	std::default_random_engine randomEngine;

	std::vector<Vector4d> samples;
	for (uint32_t i = 0; i < SSAO_SAMPLE_COUNT; i++)
	{
		Vector3d sample = { randomFloats(randomEngine) * 2.0f - 1.0f, randomFloats(randomEngine) * 2.0f - 1.0f, randomFloats(randomEngine) };
		sample.z = std::pow(sample.z, 0.5);

		float length = randomFloats(randomEngine);
		length = length * length;		// Make sample length more distributed near hemisphere center
		length = 0.1f * (1.0f - length) + 1.0f * length;

		sample = sample.Normal() * length;
		m_globalVariables.SSAOSamples[i] = Vector4d(sample, 0.0f);	// NOTE: make it 4 units to pair with gpu variable alignment
		CONVERT2SINGLE(m_globalVariables, m_singlePrecisionGlobalVariables, SSAOSamples[i]);
	}

	SetDirty();
}

bool PerBoneUniforms::Init(const std::shared_ptr<PerBoneUniforms>& pSelf)
{
	if (!ChunkBasedUniforms::Init(pSelf, sizeof(BoneData<float>)))
		return false;
	return true;
}

std::shared_ptr<PerBoneUniforms> PerBoneUniforms::Create()
{
	std::shared_ptr<PerBoneUniforms> pPerBoneUniforms = std::make_shared<PerBoneUniforms>();
	if (pPerBoneUniforms.get() && pPerBoneUniforms->Init(pPerBoneUniforms))
		return pPerBoneUniforms;
	return nullptr;
}

void PerBoneUniforms::SetBoneOffsetTransform(uint32_t chunkIndex, const DualQuaterniond& offsetDQ)
{
	m_boneData[chunkIndex].prevBoneOffsetDQ = m_boneData[chunkIndex].currBoneOffsetDQ;
	m_boneData[chunkIndex].currBoneOffsetDQ = offsetDQ;
	SetChunkDirty(chunkIndex);
}

DualQuaterniond PerBoneUniforms::GetBoneOffsetTransform(uint32_t chunkIndex) const
{
	return m_boneData[chunkIndex].currBoneOffsetDQ;
}

void PerBoneUniforms::UpdateDirtyChunkInternal(uint32_t index)
{
	m_singlePrecisionBoneData[index].currBoneOffsetDQ = m_boneData[index].currBoneOffsetDQ.SinglePrecision();
	m_singlePrecisionBoneData[index].prevBoneOffsetDQ = m_boneData[index].prevBoneOffsetDQ.SinglePrecision();
}

std::vector<UniformVarList> PerBoneUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"PerBoneUniforms",
			{
				{ Mat2x4Unit, "Bone offset transform quaternions" }
			}
		}
	};
}

uint32_t PerBoneUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));

	return bindingIndex;
}

bool BoneIndirectUniform::Init(const std::shared_ptr<BoneIndirectUniform>& pSelf, uint32_t type)
{
	if (!ChunkBasedUniforms::Init(pSelf, sizeof(uint32_t)))
		return false;

	m_boneBufferType = type;

	return true;
}

std::shared_ptr<BoneIndirectUniform> BoneIndirectUniform::Create(uint32_t type)
{
	std::shared_ptr<BoneIndirectUniform> pBoneIndirectUniform = std::make_shared<BoneIndirectUniform>();
	if (pBoneIndirectUniform.get() && pBoneIndirectUniform->Init(pBoneIndirectUniform, type))
		return pBoneIndirectUniform;
	return nullptr;
}

uint32_t BoneIndirectUniform::AllocateConsecutiveChunks(uint32_t chunkSize)
{
	uint32_t chunkIndex = ChunkBasedUniforms::AllocateConsecutiveChunks(chunkSize);
	m_boneIndexLookupTables[chunkIndex] = {};
	for (uint32_t i = chunkIndex; i < chunkIndex + chunkSize; i++)
		SetChunkDirty(i);

	return chunkIndex;
}

void BoneIndirectUniform::SetBoneTransform(uint32_t chunkIndex, std::size_t hashCode, const DualQuaterniond& offsetDQ)
{
	uint32_t boneIndex, boneChunkIndex;
	std::shared_ptr<PerBoneUniforms> pUniformBuffer = std::dynamic_pointer_cast<PerBoneUniforms>(UniformData::GetInstance()->GetUniformStorage((UniformData::UniformStorageType)m_boneBufferType));

	BoneIndexLookupTable::iterator it;

	if (!GetBoneIndex(chunkIndex, hashCode, it))
	{
		ASSERTION(pUniformBuffer != nullptr);

		boneChunkIndex = pUniformBuffer->AllocatePerObjectChunk();

		boneIndex = (uint32_t)m_boneIndexLookupTables[chunkIndex].size();
		m_boneIndexLookupTables[chunkIndex][hashCode].boneIndex = boneIndex;

		// Fill the actual mapping between bone index and bone chunk index(where bone data is located)
		m_boneChunkIndex[boneIndex + chunkIndex] = boneChunkIndex;
		SetChunkDirty(boneIndex + chunkIndex);
	}

	pUniformBuffer->SetBoneOffsetTransform(m_boneChunkIndex[boneIndex + chunkIndex], offsetDQ);
}

bool BoneIndirectUniform::GetBoneInfo(uint32_t chunkIndex, std::size_t hashCode, uint32_t& outBoneIndex, DualQuaterniond& outBoneOffsetTransformDQ)
{
	BoneIndexLookupTable::iterator it;
	if (!GetBoneIndex(chunkIndex, hashCode, it))
		return false;

	outBoneIndex = it->second.boneIndex;

	std::shared_ptr<PerBoneUniforms> pUniformBuffer = std::dynamic_pointer_cast<PerBoneUniforms>(UniformData::GetInstance()->GetUniformStorage((UniformData::UniformStorageType)m_boneBufferType));
	ASSERTION(pUniformBuffer != nullptr);

	outBoneOffsetTransformDQ = pUniformBuffer->GetBoneOffsetTransform(m_boneChunkIndex[outBoneIndex + chunkIndex]);

	return true;
}

void BoneIndirectUniform::SetBoneTransform(uint32_t chunkIndex, std::size_t hashCode, uint32_t boneIndex, const DualQuaterniond& offsetDQ)
{
	uint32_t boneChunkIndex;
	std::shared_ptr<PerBoneUniforms> pUniformBuffer = std::dynamic_pointer_cast<PerBoneUniforms>(UniformData::GetInstance()->GetUniformStorage((UniformData::UniformStorageType)m_boneBufferType));

	BoneIndexLookupTable::iterator it;
	if (!GetBoneIndex(chunkIndex, hashCode, it))
	{
		ASSERTION(pUniformBuffer != nullptr);

		boneChunkIndex = pUniformBuffer->AllocatePerObjectChunk();
		m_boneIndexLookupTables[chunkIndex][hashCode].boneIndex = boneIndex;
	}
	else
	{
		// Get bone chunk index from old bone index
		boneChunkIndex = m_boneChunkIndex[it->second.boneIndex + chunkIndex];
		it->second.boneIndex = boneIndex;
	}

	// Move data from old bone index
	m_boneChunkIndex[boneIndex + chunkIndex] = boneChunkIndex;
	SetChunkDirty(boneIndex + chunkIndex);

	pUniformBuffer->SetBoneOffsetTransform(boneChunkIndex, offsetDQ);
}

bool BoneIndirectUniform::GetBoneTransform(uint32_t chunkIndex, std::size_t hashCode, uint32_t boneIndex, DualQuaterniond& outBoneOffsetTransformDQ) const
{
	auto iter0 = m_boneIndexLookupTables.find(chunkIndex);
	ASSERTION(iter0 != m_boneIndexLookupTables.end());

	auto iter = iter0->second.find(hashCode);
	if (iter == iter0->second.end())
		return false;

	if (iter->second.boneIndex != boneIndex)
		return false;

	std::shared_ptr<PerBoneUniforms> pUniformBuffer = std::dynamic_pointer_cast<PerBoneUniforms>(UniformData::GetInstance()->GetUniformStorage((UniformData::UniformStorageType)m_boneBufferType));

	outBoneOffsetTransformDQ = pUniformBuffer->GetBoneOffsetTransform(m_boneChunkIndex[boneIndex + chunkIndex]);
	return true;
}

bool BoneIndirectUniform::GetBoneIndex(uint32_t chunkIndex, std::size_t hashCode, BoneIndexLookupTable::iterator& it)
{
	auto iter0 = m_boneIndexLookupTables.find(chunkIndex);
	ASSERTION(iter0 != m_boneIndexLookupTables.end());

	auto iter = iter0->second.find(hashCode);
	if (iter == iter0->second.end())
		return false;

	it = iter;
	return true;
}

bool BoneIndirectUniform::GetBoneCount(uint32_t chunkIndex, uint32_t& outBoneCount) const
{
	auto iter0 = m_boneIndexLookupTables.find(chunkIndex);
	ASSERTION(iter0 != m_boneIndexLookupTables.end());

	outBoneCount = (uint32_t)iter0->second.size();
	return true;
}

std::size_t BoneIndirectUniform::GetBoneHashCode(uint32_t chunkIndex, uint32_t index) const
{
	auto iter0 = m_boneIndexLookupTables.find(chunkIndex);
	for (auto it : iter0->second)
	{
		if (it.second.boneIndex == index)
			return it.first;
	}
	return 0;
}

void BoneIndirectUniform::UpdateDirtyChunkInternal(uint32_t index)
{
}

std::vector<UniformVarList> BoneIndirectUniform::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"PerMeshUniforms",
			{
				{ OneUnit, "Bone chunk indices" }
			}
		}
	};
}

uint32_t BoneIndirectUniform::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));

	return bindingIndex;
}

bool PerMeshUniforms::Init(const std::shared_ptr<PerMeshUniforms>& pSelf)
{
	if (!ChunkBasedUniforms::Init(pSelf, sizeof(MeshData)))
		return false;
	return true;
}

std::shared_ptr<PerMeshUniforms> PerMeshUniforms::Create()
{
	std::shared_ptr<PerMeshUniforms> pPerMeshUniforms = std::make_shared<PerMeshUniforms>();
	if (pPerMeshUniforms.get() && pPerMeshUniforms->Init(pPerMeshUniforms))
		return pPerMeshUniforms;
	return nullptr;
}

void PerMeshUniforms::SetBoneChunkIndexOffset(uint32_t chunkIndex, uint32_t boneChunkIndexOffset)
{
	m_meshData[chunkIndex].boneChunkIndexOffset = boneChunkIndexOffset;
	SetChunkDirty(chunkIndex);
}

void PerMeshUniforms::UpdateDirtyChunkInternal(uint32_t index)
{
}

std::vector<UniformVarList> PerMeshUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"Per Mesh Uniforms",
			{
				{ OneUnit, "Bone chunk index offset" }
			}
		}
	};
}

uint32_t PerMeshUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));

	return bindingIndex;
}


bool PerAnimationUniforms::Init(const std::shared_ptr<PerAnimationUniforms>& pSelf)
{
	if (!ChunkBasedUniforms::Init(pSelf, sizeof(AnimationData)))
		return false;
	return true;
}

std::shared_ptr<PerAnimationUniforms> PerAnimationUniforms::Create()
{
	std::shared_ptr<PerAnimationUniforms> pPerAnimationUniforms = std::make_shared<PerAnimationUniforms>();
	if (pPerAnimationUniforms.get() && pPerAnimationUniforms->Init(pPerAnimationUniforms))
		return pPerAnimationUniforms;
	return nullptr;
}

void PerAnimationUniforms::SetBoneChunkIndexOffset(uint32_t chunkIndex, uint32_t boneChunkIndexOffset)
{
	m_animationData[chunkIndex].boneChunkIndexOffset = boneChunkIndexOffset;
	SetChunkDirty(chunkIndex);
}

void PerAnimationUniforms::UpdateDirtyChunkInternal(uint32_t index)
{
}

std::vector<UniformVarList> PerAnimationUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"Per Animation Uniforms",
			{
				{ OneUnit, "Bone chunk index offset" }
			}
		}
	};
}

uint32_t PerAnimationUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));

	return bindingIndex;
}

