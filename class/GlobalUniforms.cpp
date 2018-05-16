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
	SetSSAOWindowSize({ (float)FrameBufferDiction::SSAO_WINDOW_SIZE, (float)FrameBufferDiction::SSAO_WINDOW_SIZE });
	SetBloomWindowSize({ (float)FrameBufferDiction::BLOOM_WINDOW_SIZE, (float)FrameBufferDiction::BLOOM_WINDOW_SIZE });

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
	m_globalVariables.prevProjectionMatrix = m_globalVariables.projectionMatrix;
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


void GlobalUniforms::SetSSAOWindowSize(const Vector2f& size)
{
	m_globalVariables.SSAOWindowSize.x = size.x;
	m_globalVariables.SSAOWindowSize.y = size.y;
	m_globalVariables.SSAOWindowSize.z = 1.0f / size.x;
	m_globalVariables.SSAOWindowSize.w = 1.0f / size.y;
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
					Vec4Unit,
					"MainLightDirection"
				},
				{
					Vec4Unit,
					"MainLightColor"
				},
				{
					Vec4Unit,
					"Settings: Gamma, Exposure, White Scale"
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

