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

void GlobalUniforms::UpdateUniformDataInternal()
{
	m_globalVariables.prevPN = m_globalVariables.PN;
	m_globalVariables.PN = m_globalVariables.vulkanNDC * m_globalVariables.projectionMatrix;

	m_globalVariables.DOFSettings0.z = m_globalVariables.mainCameraSettings0.w * m_globalVariables.mainCameraSettings0.w / 
		(m_globalVariables.mainCameraSettings1.y * (m_globalVariables.mainCameraSettings1.x - m_globalVariables.mainCameraSettings0.w) * m_globalVariables.mainCameraSettings0.y * 2.0f);
}

void GlobalUniforms::SetDirtyInternal()
{
}

void GlobalUniforms::SetMainLightDir(const Vector3f& dir)
{
	m_globalVariables.mainLightDir = dir.Normal();
	SetDirty();
}

void GlobalUniforms::SetMainLightColor(const Vector3f& color)
{
	m_globalVariables.mainLightColor = color;
	SetDirty();
}

void GlobalUniforms::SetMainLightVP(const Matrix4f& vp)
{
	m_globalVariables.mainLightVPN = GetVulkanNDCMatrix() * vp;
	SetDirty();
}

void GlobalUniforms::SetMainCameraSettings0(const Vector4f& settings)
{
	m_globalVariables.mainCameraSettings0 = settings;
	SetDirty();
}

void GlobalUniforms::SetMainCameraSettings1(const Vector4f& settings)
{
	m_globalVariables.mainCameraSettings1 = settings;
	SetDirty();
}

void GlobalUniforms::SetMainCameraSettings2(const Vector4f& settings)
{
	m_globalVariables.mainCameraSettings2 = settings;
	SetDirty();
}

void GlobalUniforms::SetMainCameraAspect(float aspect)
{
	m_globalVariables.mainCameraSettings0.x = aspect;
	SetDirty();
}

void GlobalUniforms::SetMainCameraFilmWidth(float filmWidth)
{
	m_globalVariables.mainCameraSettings0.y = filmWidth;
	SetDirty();
}

void GlobalUniforms::SetMainCameraFilmHeight(float filmHeight)
{
	m_globalVariables.mainCameraSettings0.z = filmHeight;
	SetDirty();
}

void GlobalUniforms::SetMainCameraFocalLength(float focalLength)
{
	m_globalVariables.mainCameraSettings0.w = focalLength;
	SetDirty();
}

void GlobalUniforms::SetMainCameraFocusDistance(float focusDistance)
{
	m_globalVariables.mainCameraSettings1.x = focusDistance;
	SetDirty();
}

void GlobalUniforms::SetMainCameraFStop(float fstop)
{
	m_globalVariables.mainCameraSettings1.y = fstop;
	SetDirty();
}

void GlobalUniforms::SetMainCameraShutterSpeed(float shutterSpeed)
{
	m_globalVariables.mainCameraSettings1.z = shutterSpeed;
	SetDirty();
}

void GlobalUniforms::SetMainCameraISO(float ISO)
{
	m_globalVariables.mainCameraSettings1.w = ISO;
	SetDirty();
}

void GlobalUniforms::SetMainCameraFarPlane(float farPlane)
{
	m_globalVariables.mainCameraSettings2.x = farPlane;
	SetDirty();
}

void GlobalUniforms::SetMainCameraHorizontalFOV(float horizontalFOV)
{
	m_globalVariables.mainCameraSettings2.y = horizontalFOV;
	SetDirty();
}

void GlobalUniforms::SetMainCameraVerticalFOV(float verticalFOV)
{
	m_globalVariables.mainCameraSettings2.z = verticalFOV;
	SetDirty();
}

void GlobalUniforms::SetMainCameraApertureDiameter(float apertureDiameter)
{
	m_globalVariables.mainCameraSettings2.w = apertureDiameter;
	SetDirty();
}

void GlobalUniforms::SetRenderSettings(const Vector4f& setting)
{
	m_globalVariables.GEW = setting;
	SetDirty();
}

void GlobalUniforms::SetSSRSettings0(const Vector4f& setting)
{
	m_globalVariables.SSRSettings0 = setting;
	SetDirty();
}

void GlobalUniforms::SetSSRSettings1(const Vector4f& setting)
{
	m_globalVariables.SSRSettings1 = setting;
	SetDirty();
}

void GlobalUniforms::SetSSRSettings2(const Vector4f& setting)
{
	m_globalVariables.SSRSettings2 = setting;
	SetDirty();
}

void GlobalUniforms::SetBRDFBias(float BRDFBias)
{
	m_globalVariables.SSRSettings0.x = BRDFBias;
	SetDirty();
}

void GlobalUniforms::SetSSRMip(float SSRMip)
{
	m_globalVariables.SSRSettings0.y = SSRMip;
	SetDirty();
}

void GlobalUniforms::SetSampleNormalRegenCount(float count)
{
	m_globalVariables.SSRSettings0.z = count;
	SetDirty();
}

void GlobalUniforms::SetSampleNormalRegenMargin(float margin)
{
	m_globalVariables.SSRSettings0.w = margin;
	SetDirty();
}

void GlobalUniforms::SetSSRTStride(float stride)
{
	m_globalVariables.SSRSettings1.x = stride;
	SetDirty();
}

void GlobalUniforms::SetSSRTInitOffset(float offset)
{
	m_globalVariables.SSRSettings1.y = offset;
	SetDirty();
}

void GlobalUniforms::SetMaxSSRTStepCount(float count)
{
	m_globalVariables.SSRSettings1.z = count;
	SetDirty();
}

void GlobalUniforms::SetSSRTThickness(float thickness)
{
	m_globalVariables.SSRSettings1.w = thickness;
	SetDirty();
}

void GlobalUniforms::SetSSRTBorderFadingDist(float dist)
{
	m_globalVariables.SSRSettings2.x = dist;
	SetDirty();
}

void GlobalUniforms::SetSSRTStepCountFadingDist(float dist)
{
	m_globalVariables.SSRSettings2.y = dist;
	SetDirty();
}

void GlobalUniforms::SetScreenSizeMipLevel(float mipLevel)
{
	m_globalVariables.SSRSettings2.z = mipLevel;
	SetDirty();
}

void GlobalUniforms::SetTemporalSettings0(const Vector4f& setting)
{
	m_globalVariables.TemporalSettings0 = setting;
	SetDirty();
}

void GlobalUniforms::SetTemporalSettings1(const Vector4f& setting)
{
	m_globalVariables.TemporalSettings1 = setting;
	SetDirty();
}

void GlobalUniforms::SetPrevMotionImpact(float prevMotionImpact)
{
	m_globalVariables.TemporalSettings0.x = prevMotionImpact;
	SetDirty();
}

void GlobalUniforms::SetCurrMotionImpact(float currMotionImpact)
{
	m_globalVariables.TemporalSettings0.y = currMotionImpact;
	SetDirty();
}

void GlobalUniforms::SetMaxClippedPrevRatio(float maxClippedPrevRatio)
{
	m_globalVariables.TemporalSettings0.z = maxClippedPrevRatio;
	SetDirty();
}

void GlobalUniforms::SetMotionImpactLowerBound(float motionImpactLowerBound)
{
	m_globalVariables.TemporalSettings1.x = motionImpactLowerBound;
	SetDirty();
}

void GlobalUniforms::SetMotionImpactUpperBound(float motionImpactUpperBound)
{
	m_globalVariables.TemporalSettings1.y = motionImpactUpperBound;
	SetDirty();
}

void GlobalUniforms::SetBloomSettings0(const Vector4f& setting)
{
	m_globalVariables.BloomSettings0 = setting;
	SetDirty();
}

void GlobalUniforms::SetBloomSettings1(const Vector4f& setting)
{
	m_globalVariables.BloomSettings1 = setting;
	SetDirty();
}

void GlobalUniforms::SetBloomClampingLowerBound(float lowerBound)
{
	m_globalVariables.BloomSettings0.x = lowerBound;
	SetDirty();
}

void GlobalUniforms::SetBloomClampingUpperBound(float upperBound)
{
	m_globalVariables.BloomSettings0.y = upperBound;
	SetDirty();
}

void GlobalUniforms::SetUpsampleScale(float upsampleScale)
{
	m_globalVariables.BloomSettings0.z = upsampleScale;
	SetDirty();
}

void GlobalUniforms::SetBloomAmplify(float bloomAmplify)
{
	m_globalVariables.BloomSettings1.x = bloomAmplify;
	SetDirty();
}

void GlobalUniforms::SetBloomSlope(float bloomSlope)
{
	m_globalVariables.BloomSettings1.y = bloomSlope;
	SetDirty();
}

void GlobalUniforms::SetMaxCOC(float maxCOC)
{
	m_globalVariables.DOFSettings0.x = maxCOC;
	m_globalVariables.DOFSettings0.y = 1 / maxCOC;
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

bool PerBoneUniforms::Init(const std::shared_ptr<PerBoneUniforms>& pSelf)
{
	if (!ChunkBasedUniforms::Init(pSelf, sizeof(BoneData)))
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

void PerBoneUniforms::SetBoneOffsetTransform(uint32_t chunkIndex, const DualQuaternionf& offsetDQ)
{
	 m_boneData[chunkIndex].boneOffsetDQ = offsetDQ;
	 SetChunkDirty(chunkIndex);
}

DualQuaternionf PerBoneUniforms::GetBoneOffsetTransform(uint32_t chunkIndex) const
{
	return m_boneData[chunkIndex].boneOffsetDQ;
}

void PerBoneUniforms::UpdateDirtyChunkInternal(uint32_t index)
{
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

void BoneIndirectUniform::SetBoneTransform(uint32_t chunkIndex, const std::wstring& boneName, const DualQuaternionf& offsetDQ)
{
	uint32_t boneIndex, boneChunkIndex;
	std::shared_ptr<PerBoneUniforms> pUniformBuffer = std::dynamic_pointer_cast<PerBoneUniforms>(UniformData::GetInstance()->GetUniformStorage((UniformData::UniformStorageType)m_boneBufferType));

	if (!GetBoneIndex(chunkIndex, boneName, boneIndex))
	{
		ASSERTION(pUniformBuffer != nullptr);

		boneChunkIndex = pUniformBuffer->AllocatePerObjectChunk();

		boneIndex = m_boneIndexLookupTables[chunkIndex].size();
		m_boneIndexLookupTables[chunkIndex][boneName] = boneIndex;

		// Fill the actual mapping between bone index and bone chunk index(where bone data is located)
		m_boneChunkIndex[boneIndex + chunkIndex] = boneChunkIndex;
		SetChunkDirty(boneIndex + chunkIndex);
	}

	pUniformBuffer->SetBoneOffsetTransform(boneChunkIndex, offsetDQ);
}

bool BoneIndirectUniform::GetBoneTransform(uint32_t chunkIndex, const std::wstring& boneName, DualQuaternionf& outBoneOffsetTransformDQ) const
{
	uint32_t boneIndex;

	if (!GetBoneIndex(chunkIndex, boneName, boneIndex))
		return false;

	std::shared_ptr<PerBoneUniforms> pUniformBuffer = std::dynamic_pointer_cast<PerBoneUniforms>(UniformData::GetInstance()->GetUniformStorage((UniformData::UniformStorageType)m_boneBufferType));
	ASSERTION(pUniformBuffer != nullptr);

	outBoneOffsetTransformDQ = pUniformBuffer->GetBoneOffsetTransform(boneIndex + chunkIndex);

	return true;
}

void BoneIndirectUniform::SetBoneTransform(uint32_t chunkIndex, const std::wstring& boneName, uint32_t boneIndex, const DualQuaternionf& offsetDQ)
{
	uint32_t existBoneIndex, boneChunkIndex;
	std::shared_ptr<PerBoneUniforms> pUniformBuffer = std::dynamic_pointer_cast<PerBoneUniforms>(UniformData::GetInstance()->GetUniformStorage((UniformData::UniformStorageType)m_boneBufferType));

	if (!GetBoneIndex(chunkIndex, boneName, existBoneIndex))
	{
		ASSERTION(pUniformBuffer != nullptr);

		boneChunkIndex = pUniformBuffer->AllocatePerObjectChunk();
	}
	else
		// Get bone chunk index from old bone index
		boneChunkIndex = m_boneChunkIndex[existBoneIndex + chunkIndex];

	// Refresh lookup table
	m_boneIndexLookupTables[chunkIndex][boneName] = boneIndex;

	// Move data from old bone index
	m_boneChunkIndex[boneIndex + chunkIndex] = boneChunkIndex;
	SetChunkDirty(boneIndex + chunkIndex);

	pUniformBuffer->SetBoneOffsetTransform(boneChunkIndex, offsetDQ);
}

bool BoneIndirectUniform::GetBoneTransform(uint32_t chunkIndex, const std::wstring& boneName, uint32_t boneIndex, DualQuaternionf& outBoneOffsetTransformDQ) const
{
	auto iter0 = m_boneIndexLookupTables.find(chunkIndex);
	ASSERTION(iter0 != m_boneIndexLookupTables.end());

	auto iter = iter0->second.find(boneName);
	if (iter == iter0->second.end())
		return false;

	if (iter->second != boneIndex)
		return false;

	std::shared_ptr<PerBoneUniforms> pUniformBuffer = std::dynamic_pointer_cast<PerBoneUniforms>(UniformData::GetInstance()->GetUniformStorage((UniformData::UniformStorageType)m_boneBufferType));

	outBoneOffsetTransformDQ = pUniformBuffer->GetBoneOffsetTransform(m_boneChunkIndex[boneIndex + chunkIndex]);
	return true;
}

bool BoneIndirectUniform::GetBoneIndex(uint32_t chunkIndex, const std::wstring& boneName, uint32_t& outBoneIndex) const
{
	auto iter0 = m_boneIndexLookupTables.find(chunkIndex);
	ASSERTION(iter0 != m_boneIndexLookupTables.end());

	auto iter = iter0->second.find(boneName);
	if (iter == iter0->second.end())
		return false;

	outBoneIndex = iter->second;
	return true;
}

bool BoneIndirectUniform::GetBoneCount(uint32_t chunkIndex, uint32_t& outBoneCount) const
{
	auto iter0 = m_boneIndexLookupTables.find(chunkIndex);
	ASSERTION(iter0 != m_boneIndexLookupTables.end());

	outBoneCount = iter0->second.size();
	return true;
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

