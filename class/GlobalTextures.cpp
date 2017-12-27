#include "../vulkan/Texture2DArray.h"
#include "../vulkan/Texture2D.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/TextureCube.h"
#include "../vulkan/CommandPool.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/GlobalVulkanStates.h"
#include "../vulkan/Queue.h"
#include "../vulkan/StagingBufferManager.h"
#include "../vulkan/Framebuffer.h"
#include "../class/RenderWorkManager.h"
#include "../class/Mesh.h"
#include "../component/MeshRenderer.h"
#include "../component/Camera.h"
#include "../Base/BaseObject.h"
#include "../scene/SceneGenerator.h"
#include "GlobalTextures.h"
#include "Material.h"

bool GlobalTextures::Init(const std::shared_ptr<GlobalTextures>& pSelf)
{
	if (!SelfRefBase<GlobalTextures>::Init(pSelf))
		return false;

	InitTextureDiction();
	InitIBLTextures();

	return true;
}

// FIXME: Make it configurable future
void GlobalTextures::InitTextureDiction()
{
	m_textureDiction.resize(InGameTextureTypeCount);

	m_textureDiction[RGBA8_1024].textureArrayName = "RGBA8TextureArray";
	m_textureDiction[RGBA8_1024].textureArrayDescription = "RGBA8, size16, mipLevel11";
	m_textureDiction[RGBA8_1024].pTextureArray = Texture2DArray::CreateEmptyTexture2DArray(GetDevice(), 1024, 1024, std::log2(1024) + 1, 16, VK_FORMAT_R8G8B8A8_UNORM);
	m_textureDiction[RGBA8_1024].maxSlotIndex = 0;
	m_textureDiction[RGBA8_1024].currentEmptySlot = 0;

	m_textureDiction[R8_1024].textureArrayName = "R8TextureArray";
	m_textureDiction[R8_1024].textureArrayDescription = "R8, size16, mipLevel11";
	m_textureDiction[R8_1024].pTextureArray = Texture2DArray::CreateEmptyTexture2DArray(GetDevice(), 1024, 1024, std::log2(1024) + 1, 16, VK_FORMAT_R8_UNORM);
	m_textureDiction[R8_1024].maxSlotIndex = 0;
	m_textureDiction[R8_1024].currentEmptySlot = 0;
}

void GlobalTextures::InitIBLTextures()
{
	m_IBLCubeTextures.resize(IBLCubeTextureTypeCount);
	m_IBLCubeTextures[RGBA16_1024_SkyBox] = TextureCube::CreateEmptyTextureCube(GetDevice(), 1024, 1024, std::log2(1024) + 1, VK_FORMAT_R16G16B16A16_SFLOAT);
	m_IBLCubeTextures[RGBA16_512_SkyBoxIrradiance] = TextureCube::CreateEmptyTextureCube(GetDevice(), OFFSCREEN_SIZE, OFFSCREEN_SIZE, 1, VK_FORMAT_R16G16B16A16_SFLOAT);
	m_IBLCubeTextures[RGBA16_512_SkyBoxPrefilterEnv] = TextureCube::CreateEmptyTextureCube(GetDevice(), OFFSCREEN_SIZE, OFFSCREEN_SIZE, std::log2(512) + 1, VK_FORMAT_R16G16B16A16_SFLOAT);

	m_IBL2DTextures.resize(IBL2DTextureTypeCount);
	m_IBL2DTextures[RGBA16_512_BRDFLut] = Texture2D::CreateEmptyTexture(GetDevice(), OFFSCREEN_SIZE, OFFSCREEN_SIZE, VK_FORMAT_R16G16B16A16_SFLOAT);
}

void GlobalTextures::InitIBLTextures(const gli::texture_cube& skyBoxTex)
{
	m_IBLCubeTextures[RGBA16_1024_SkyBox]->UpdateByteStream({ {skyBoxTex} });
	InitIrradianceTexture();
	InitPrefilterEnvTexture();
	InitBRDFLUTTexture();
}

void GlobalTextures::InitIrradianceTexture()
{
	std::shared_ptr<FrameBuffer> pEnvFrameBuffer = FrameBuffer::CreateOffScreenFrameBuffer(GetDevice(), OFFSCREEN_SIZE, OFFSCREEN_SIZE, RenderWorkManager::GetInstance()->GetDefaultOffscreenRenderPass());
	RenderWorkManager::GetInstance()->SetDefaultOffscreenRenderPass(pEnvFrameBuffer);

	SceneGenerator::GetInstance()->GenerateIrradianceGenScene();

	RenderWorkManager::GetInstance()->SetRenderState(RenderWorkManager::IrradianceGen);

	Vector3f up = { 0, 1, 0 };
	Vector3f look = { 0, 0, -1 };
	look.Normalize();
	Vector3f xaxis = up ^ look.Negativate();
	xaxis.Normalize();
	Vector3f yaxis = look ^ xaxis;
	yaxis.Normalize();

	Matrix3f rotation;
	rotation.c[0] = xaxis;
	rotation.c[1] = yaxis;
	rotation.c[2] = look;

	Matrix3f cameraRotations[] =
	{
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * Matrix3f::Rotation(3.0 * 3.14159265 / 2.0, Vector3f(0, 1, 0)) * rotation,	// Positive X, i.e right
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * Matrix3f::Rotation(1.0 * 3.14159265 / 2.0, Vector3f(0, 1, 0)) * rotation,	// Negative X, i.e left
		Matrix3f::Rotation(3.0 * 3.14159265 / 2.0, Vector3f(1, 0, 0)) * rotation,	// Positive Y, i.e top
		Matrix3f::Rotation(1.0 * 3.14159265 / 2.0, Vector3f(1, 0, 0)) * rotation,	// Negative Y, i.e bottom
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * rotation,	// Positive Z, i.e back
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(0, 0, 1)) * rotation,	// Negative Z, i.e front
	};

	for (uint32_t i = 0; i < 6; i++)
	{
		std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();

		std::vector<VkClearValue> clearValues =
		{
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 1.0f, 0 }
		};

		VkViewport viewport =
		{
			0, 0,
			OFFSCREEN_SIZE, OFFSCREEN_SIZE,
			0, 1
		};

		VkRect2D scissorRect =
		{
			0, 0,
			OFFSCREEN_SIZE, OFFSCREEN_SIZE,
		};

		GetGlobalVulkanStates()->SetViewport(viewport);
		GetGlobalVulkanStates()->SetScissorRect(scissorRect);

		SceneGenerator::GetInstance()->GetCameraObject()->SetRotation(cameraRotations[i]);

		pDrawCmdBuffer->StartPrimaryRecording();

		pDrawCmdBuffer->BeginRenderPass(RenderWorkManager::GetInstance()->GetCurrentFrameBuffer(), RenderWorkManager::GetInstance()->GetCurrentRenderPass(), clearValues, true);

		SceneGenerator::GetInstance()->GetMaterial0()->OnFrameStart();

		SceneGenerator::GetInstance()->GetRootObject()->Update();
		SceneGenerator::GetInstance()->GetRootObject()->LateUpdate();
		UniformData::GetInstance()->SyncDataBuffer();
		SceneGenerator::GetInstance()->GetMaterial0()->Draw();

		SceneGenerator::GetInstance()->GetMaterial0()->OnFrameEnd();

		RenderWorkManager::GetInstance()->GetCurrentRenderPass()->ExecuteCachedSecondaryCommandBuffers(pDrawCmdBuffer);

		pDrawCmdBuffer->EndRenderPass();


		pDrawCmdBuffer->EndPrimaryRecording();

		GlobalGraphicQueue()->SubmitCommandBuffer(pDrawCmdBuffer, nullptr, true);

		pEnvFrameBuffer->ExtractContent(m_IBLCubeTextures[RGBA16_512_SkyBoxIrradiance], 0, 1, i, 1);
	}
}

void GlobalTextures::InitPrefilterEnvTexture()
{
	std::shared_ptr<FrameBuffer> pEnvFrameBuffer = FrameBuffer::CreateOffScreenFrameBuffer(GetDevice(), OFFSCREEN_SIZE, OFFSCREEN_SIZE, RenderWorkManager::GetInstance()->GetDefaultOffscreenRenderPass());
	RenderWorkManager::GetInstance()->SetDefaultOffscreenRenderPass(pEnvFrameBuffer);

	SceneGenerator::GetInstance()->GeneratePrefilterEnvGenScene();

	RenderWorkManager::GetInstance()->SetRenderState(RenderWorkManager::ReflectionGen);

	Vector3f up = { 0, 1, 0 };
	Vector3f look = { 0, 0, -1 };
	look.Normalize();
	Vector3f xaxis = up ^ look.Negativate();
	xaxis.Normalize();
	Vector3f yaxis = look ^ xaxis;
	yaxis.Normalize();

	Matrix3f rotation;
	rotation.c[0] = xaxis;
	rotation.c[1] = yaxis;
	rotation.c[2] = look;

	Matrix3f cameraRotations[] =
	{
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * Matrix3f::Rotation(3.0 * 3.14159265 / 2.0, Vector3f(0, 1, 0)) * rotation,	// Positive X, i.e right
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * Matrix3f::Rotation(1.0 * 3.14159265 / 2.0, Vector3f(0, 1, 0)) * rotation,	// Negative X, i.e left
		Matrix3f::Rotation(3.0 * 3.14159265 / 2.0, Vector3f(1, 0, 0)) * rotation,	// Positive Y, i.e top
		Matrix3f::Rotation(1.0 * 3.14159265 / 2.0, Vector3f(1, 0, 0)) * rotation,	// Negative Y, i.e bottom
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(1, 0, 0)) * rotation,	// Positive Z, i.e back
		Matrix3f::Rotation(1.0 * 3.14159265 / 1.0, Vector3f(0, 0, 1)) * rotation,	// Negative Z, i.e front
	};

	uint32_t mipLevels = std::log2(OFFSCREEN_SIZE);
	for (uint32_t mipLevel = 0; mipLevel < mipLevels + 1; mipLevel++)
	{
		UniformData::GetInstance()->GetPerFrameUniforms()->SetPadding(mipLevel / (float)mipLevels);
		uint32_t size = std::pow(2, mipLevels - mipLevel);
		for (uint32_t i = 0; i < 6; i++)
		{
			std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();

			std::vector<VkClearValue> clearValues =
			{
				{ 0.0f, 0.0f, 0.0f, 0.0f },
				{ 1.0f, 0 }
			};

			VkViewport viewport =
			{
				0, 0,
				size, size,
				0, 1
			};

			VkRect2D scissorRect =
			{
				0, 0,
				size, size,
			};

			GetGlobalVulkanStates()->SetViewport(viewport);
			GetGlobalVulkanStates()->SetScissorRect(scissorRect);

			SceneGenerator::GetInstance()->GetCameraObject()->SetRotation(cameraRotations[i]);

			pDrawCmdBuffer->StartPrimaryRecording();

			pDrawCmdBuffer->BeginRenderPass(RenderWorkManager::GetInstance()->GetCurrentFrameBuffer(), RenderWorkManager::GetInstance()->GetCurrentRenderPass(), clearValues, true);

			SceneGenerator::GetInstance()->GetMaterial0()->OnFrameStart();

			SceneGenerator::GetInstance()->GetRootObject()->Update();
			SceneGenerator::GetInstance()->GetRootObject()->LateUpdate();
			UniformData::GetInstance()->SyncDataBuffer();
			SceneGenerator::GetInstance()->GetMaterial0()->Draw();

			SceneGenerator::GetInstance()->GetMaterial0()->OnFrameEnd();

			RenderWorkManager::GetInstance()->GetCurrentRenderPass()->ExecuteCachedSecondaryCommandBuffers(pDrawCmdBuffer);

			pDrawCmdBuffer->EndRenderPass();

			pDrawCmdBuffer->EndPrimaryRecording();

			GlobalGraphicQueue()->SubmitCommandBuffer(pDrawCmdBuffer, nullptr, true);

			pEnvFrameBuffer->ExtractContent(m_IBLCubeTextures[RGBA16_512_SkyBoxPrefilterEnv], mipLevel, 1, i, 1, size, size);
		}
	}
}

void GlobalTextures::InitBRDFLUTTexture()
{
	std::shared_ptr<FrameBuffer> pEnvFrameBuffer = FrameBuffer::CreateOffScreenFrameBuffer(GetDevice(), OFFSCREEN_SIZE, OFFSCREEN_SIZE, RenderWorkManager::GetInstance()->GetDefaultOffscreenRenderPass());
	RenderWorkManager::GetInstance()->SetDefaultOffscreenRenderPass(pEnvFrameBuffer);

	SceneGenerator::GetInstance()->GenerateBRDFLUTGenScene();

	RenderWorkManager::GetInstance()->SetRenderState(RenderWorkManager::BrdfLutGen);

	std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();

	std::vector<VkClearValue> clearValues =
	{
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0 }
	};

	VkViewport viewport =
	{
		0, 0,
		OFFSCREEN_SIZE, OFFSCREEN_SIZE,
		0, 1
	};

	VkRect2D scissorRect =
	{
		0, 0,
		OFFSCREEN_SIZE, OFFSCREEN_SIZE,
	};

	pDrawCmdBuffer->StartPrimaryRecording();

	GetGlobalVulkanStates()->SetViewport(viewport);
	GetGlobalVulkanStates()->SetScissorRect(scissorRect);

	pDrawCmdBuffer->BeginRenderPass(RenderWorkManager::GetInstance()->GetCurrentFrameBuffer(), RenderWorkManager::GetInstance()->GetCurrentRenderPass(), clearValues, true);

	SceneGenerator::GetInstance()->GetRootObject()->Update();
	SceneGenerator::GetInstance()->GetRootObject()->LateUpdate();
	UniformData::GetInstance()->SyncDataBuffer();
	SceneGenerator::GetInstance()->GetMaterial0()->Draw();

	RenderWorkManager::GetInstance()->GetCurrentRenderPass()->ExecuteCachedSecondaryCommandBuffers(pDrawCmdBuffer);

	pDrawCmdBuffer->EndRenderPass();


	pDrawCmdBuffer->EndPrimaryRecording();

	GlobalGraphicQueue()->SubmitCommandBuffer(pDrawCmdBuffer, nullptr, true);

	pEnvFrameBuffer->ExtractContent(m_IBL2DTextures[RGBA16_512_BRDFLut]);
}

std::shared_ptr<GlobalTextures> GlobalTextures::Create()
{
	std::shared_ptr<GlobalTextures> pGlobalTextures = std::make_shared<GlobalTextures>();
	if (pGlobalTextures.get() && pGlobalTextures->Init(pGlobalTextures))
		return pGlobalTextures;
	return nullptr;
}

std::vector<UniformVarList> GlobalTextures::PrepareUniformVarList()
{
	return 
	{
		{
			CombinedSampler,
			"RGBA8_1024_Texture_Array"
		},
		{
			CombinedSampler,
			"R8_1024_Texture_Array"
		},
		{
			CombinedSampler,
			"RGBA8_1024_Texture_Cube_SkyBox"
		},
		{
			CombinedSampler,
			"R8_512_Texture_Cube_Irradiance"
		},
		{
			CombinedSampler,
			"RGBA8_512_Texture_Cube_PrefilterEnv"
		},
		{
			CombinedSampler,
			"R8_512_Texture_2D_BRDFLUT"
		}
	};
}

void GlobalTextures::InsertTexture(InGameTextureType type, const TextureDesc& desc, const gli::texture2d& gliTexture2d)
{
	uint32_t emptySlot = m_textureDiction[type].currentEmptySlot;
	m_textureDiction[type].textureDescriptions[emptySlot] = desc;
	m_textureDiction[type].pTextureArray->InsertTexture(gliTexture2d, emptySlot);

	// Find if there's an available slot within the pool
	bool found = false;
	for (uint32_t i = 0; i < m_textureDiction[type].maxSlotIndex; i++)
	{
		if (m_textureDiction[type].textureDescriptions.find(i) == m_textureDiction[type].textureDescriptions.end())
		{
			m_textureDiction[type].currentEmptySlot = i;
			found = true;
			break;
		}
	}

	// If there's no available slot within, then increase slot count by 1 and assign empty slot to it
	if (!found)
	{
		m_textureDiction[type].currentEmptySlot = m_textureDiction[type].maxSlotIndex + 1;
		m_textureDiction[type].maxSlotIndex = m_textureDiction[type].currentEmptySlot;
	}
}

