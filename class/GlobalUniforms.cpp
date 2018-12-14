#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/Image.h"
#include "../vulkan/Texture2D.h"
#include "../vulkan/Texture2DArray.h"
#include "../vulkan/TextureCube.h"
#include "GlobalTextures.h"
#include "GlobalUniforms.h"
#include "Material.h"
#include <random>

bool GlobalUniforms::Init(const std::shared_ptr<GlobalUniforms>& pSelf)
{
	if (!UniformDataStorage::Init(pSelf, sizeof(m_globalVariables), false))
		return false;

	// NDC space Y axis is reversed in Vulkan compared to OpenGL
	m_globalVariables.vulkanNDC.c[1].y = -1.0f;

	// NDC space z ranges from 0 to 1 in Vulkan compared to OpenGL's -1 to 1
	m_globalVariables.vulkanNDC.c[2].z = m_globalVariables.vulkanNDC.c[3].z = 0.5f;

	SetGameWindowSize({ (float)GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, (float)GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height });
	SetEnvGenWindowSize({ (float)FrameBufferDiction::ENV_GEN_WINDOW_SIZE, (float)FrameBufferDiction::ENV_GEN_WINDOW_SIZE });
	SetShadowGenWindowSize({ (float)FrameBufferDiction::SHADOW_GEN_WINDOW_SIZE, (float)FrameBufferDiction::SHADOW_GEN_WINDOW_SIZE });
	SetSSAOSSRWindowSize({ (float)FrameBufferDiction::SSAO_SSR_WINDOW_WIDTH, (float)FrameBufferDiction::SSAO_SSR_WINDOW_HEIGHT });
	SetBloomWindowSize({ (float)FrameBufferDiction::BLOOM_WINDOW_SIZE, (float)FrameBufferDiction::BLOOM_WINDOW_SIZE });
	SetMotionTileSize({ (float)FrameBufferDiction::MOTION_TILE_SIZE, (float)FrameBufferDiction::MOTION_TILE_SIZE });

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

void GlobalUniforms::SetProjectionMatrix(const Matrix4f& proj) 
{ 
	// Since prev proj matrix is used to calculate per-pixel vocelity buffer,
	// we need to handle the impact of camera jitter for temporal.
	// Using the same proj matrix for both prev and curr, which behaves exactly what we want here
	m_globalVariables.prevProjectionMatrix = proj;
	m_globalVariables.projectionMatrix = proj;
	SetDirty();
}

void GlobalUniforms::SetVulkanNDCMatrix(const Matrix4f& vndc) 
{ 
	m_globalVariables.vulkanNDC = vndc;
	SetDirty();
}

void GlobalUniforms::SetGameWindowSize(const Vector2f& size)
{
	m_globalVariables.gameWindowSize.x = size.x;
	m_globalVariables.gameWindowSize.y = size.y;
	m_globalVariables.gameWindowSize.z = 1.0f / size.x;
	m_globalVariables.gameWindowSize.w = 1.0f / size.y;
	SetDirty();
}

void GlobalUniforms::SetEnvGenWindowSize(const Vector2f& size)
{
	m_globalVariables.envGenWindowSize.x = size.x;
	m_globalVariables.envGenWindowSize.y = size.y;
	m_globalVariables.envGenWindowSize.z = 1.0f / size.x;
	m_globalVariables.envGenWindowSize.w = 1.0f / size.y;
	SetDirty();
}

void GlobalUniforms::SetShadowGenWindowSize(const Vector2f& size)
{
	m_globalVariables.shadowGenWindowSize.x = size.x;
	m_globalVariables.shadowGenWindowSize.y = size.y;
	m_globalVariables.shadowGenWindowSize.z = 1.0f / size.x;
	m_globalVariables.shadowGenWindowSize.w = 1.0f / size.y;
	SetDirty();
}


void GlobalUniforms::SetSSAOSSRWindowSize(const Vector2f& size)
{
	m_globalVariables.SSAOSSRWindowSize.x = size.x;
	m_globalVariables.SSAOSSRWindowSize.y = size.y;
	m_globalVariables.SSAOSSRWindowSize.z = 1.0f / size.x;
	m_globalVariables.SSAOSSRWindowSize.w = 1.0f / size.y;
	SetDirty();
}

void GlobalUniforms::SetBloomWindowSize(const Vector2f& size)
{
	m_globalVariables.bloomWindowSize.x = size.x;
	m_globalVariables.bloomWindowSize.y = size.y;
	m_globalVariables.bloomWindowSize.z = 1.0f / size.x;
	m_globalVariables.bloomWindowSize.w = 1.0f / size.y;
	SetDirty();
}

void GlobalUniforms::SetMotionTileSize(const Vector2f& size)
{
	m_globalVariables.motionTileWindowSize.x = size.x;
	m_globalVariables.motionTileWindowSize.y = size.y;
	m_globalVariables.motionTileWindowSize.z = (uint32_t)m_globalVariables.gameWindowSize.x / (uint32_t)size.x + (((uint32_t)m_globalVariables.gameWindowSize.x % (uint32_t)size.x) > 0 ? 1 : 0);
	m_globalVariables.motionTileWindowSize.w = (uint32_t)m_globalVariables.gameWindowSize.y / (uint32_t)size.y + (((uint32_t)m_globalVariables.gameWindowSize.y % (uint32_t)size.y) > 0 ? 1 : 0);
	SetDirty();
}

void GlobalUniforms::SyncBufferDataInternal()
{
	m_globalVariables.prevPN = m_globalVariables.PN;
	m_globalVariables.PN = m_globalVariables.vulkanNDC * m_globalVariables.projectionMatrix;

	GetBuffer()->UpdateByteStream(&m_globalVariables, FrameMgr()->FrameIndex() * GetFrameOffset(), sizeof(m_globalVariables));
}

void GlobalUniforms::SetDirty()
{
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetMainLightDir(const Vector3f& dir)
{
	m_globalVariables.mainLightDir = dir.Normal();
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetMainLightColor(const Vector3f& color)
{
	m_globalVariables.mainLightColor = color;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetMainLightVP(const Matrix4f& vp)
{
	m_globalVariables.mainLightVPN = GetVulkanNDCMatrix() * vp;
	UniformDataStorage::SetDirty();
}


void GlobalUniforms::SetRenderSettings(const Vector4f& setting)
{
	m_globalVariables.GEW = setting;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetSSRSettings0(const Vector4f& setting)
{
	m_globalVariables.SSRSettings0 = setting;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetSSRSettings1(const Vector4f& setting)
{
	m_globalVariables.SSRSettings1 = setting;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetSSRSettings2(const Vector4f& setting)
{
	m_globalVariables.SSRSettings2 = setting;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetBRDFBias(float BRDFBias)
{
	m_globalVariables.SSRSettings0.x = BRDFBias;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetSSRMip(float SSRMip)
{
	m_globalVariables.SSRSettings0.y = SSRMip;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetSampleNormalRegenCount(float count)
{
	m_globalVariables.SSRSettings0.z = count;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetSampleNormalRegenMargin(float margin)
{
	m_globalVariables.SSRSettings0.w = margin;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetSSRTStride(float stride)
{
	m_globalVariables.SSRSettings1.x = stride;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetSSRTInitOffset(float offset)
{
	m_globalVariables.SSRSettings1.y = offset;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetMaxSSRTStepCount(float count)
{
	m_globalVariables.SSRSettings1.z = count;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetSSRTThickness(float thickness)
{
	m_globalVariables.SSRSettings1.w = thickness;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetSSRTBorderFadingDist(float dist)
{
	m_globalVariables.SSRSettings2.x = dist;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetSSRTStepCountFadingDist(float dist)
{
	m_globalVariables.SSRSettings2.y = dist;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetScreenSizeMipLevel(float mipLevel)
{
	m_globalVariables.SSRSettings2.z = mipLevel;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetTemporalSettings0(const Vector4f& setting)
{
	m_globalVariables.TemporalSettings0 = setting;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetTemporalSettings1(const Vector4f& setting)
{
	m_globalVariables.TemporalSettings1 = setting;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetPrevMotionImpact(float prevMotionImpact)
{
	m_globalVariables.TemporalSettings0.x = prevMotionImpact;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetCurrMotionImpact(float currMotionImpact)
{
	m_globalVariables.TemporalSettings0.y = currMotionImpact;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetMaxClippedPrevRatio(float maxClippedPrevRatio)
{
	m_globalVariables.TemporalSettings0.z = maxClippedPrevRatio;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetMotionImpactLowerBound(float motionImpactLowerBound)
{
	m_globalVariables.TemporalSettings1.x = motionImpactLowerBound;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetMotionImpactUpperBound(float motionImpactUpperBound)
{
	m_globalVariables.TemporalSettings1.y = motionImpactUpperBound;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetBloomSettings0(const Vector4f& setting)
{
	m_globalVariables.BloomSettings0 = setting;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetBloomSettings1(const Vector4f& setting)
{
	m_globalVariables.BloomSettings1 = setting;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetBloomClampingLowerBound(float lowerBound)
{
	m_globalVariables.BloomSettings0.x = lowerBound;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetBloomClampingUpperBound(float upperBound)
{
	m_globalVariables.BloomSettings0.y = upperBound;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetUpsampleScale(float upsampleScale)
{
	m_globalVariables.BloomSettings0.z = upsampleScale;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetBloomAmplify(float bloomAmplify)
{
	m_globalVariables.BloomSettings1.x = bloomAmplify;
	UniformDataStorage::SetDirty();
}

void GlobalUniforms::SetBloomSlope(float bloomSlope)
{
	m_globalVariables.BloomSettings1.y = bloomSlope;
	UniformDataStorage::SetDirty();
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
					"VulkanNDCMatrix"
				},
				{
					Mat4Unit,
					"ProjectionVulkanNDC"
				},
				{
					Mat4Unit,
					"PrevProjectionMatrix"
				},
				{
					Mat4Unit,
					"PrevPN"
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
					"MainLightVPN"
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
					"Temporal Settings1"
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
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine randomEngine;

	std::vector<Vector4f> samples;
	for (uint32_t i = 0; i < SSAO_SAMPLE_COUNT; i++)
	{
		Vector3f sample = { randomFloats(randomEngine) * 2.0f - 1.0f, randomFloats(randomEngine) * 2.0f - 1.0f, randomFloats(randomEngine) };

		float length = randomFloats(randomEngine);
		length = length * length;		// Make sample length more distributed near hemisphere center
		length = 0.1f * (1.0f - length) + 1.0f * length;

		sample = sample.Normal() * length;
		m_globalVariables.SSAOSamples[i] = Vector4f(sample, 0.0f);	// NOTE: make it 4 units to pair with gpu variable alignment
	}

	SetDirty();
}

