#include "../vulkan/Image.h"
#include "../vulkan/Image.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Image.h"
#include "../vulkan/CommandPool.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/GlobalVulkanStates.h"
#include "../vulkan/Queue.h"
#include "../vulkan/StagingBufferManager.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/Fence.h"
#include "../vulkan/SwapChainImage.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/PerFrameResource.h"
#include "../vulkan/Semaphore.h"
#include "FrameWorkManager.h"
#include "../class/RenderWorkManager.h"
#include "../class/Mesh.h"
#include "../component/MeshRenderer.h"
#include "../Base/BaseObject.h"
#include "../scene/SceneGenerator.h"
#include "../class/RenderPassDiction.h"
#include "../vulkan/DescriptorSet.h"
#include "../thread/ThreadTaskQueue.hpp"
#include "ForwardRenderPass.h"
#include "GlobalTextures.h"
#include "Material.h"
#include "ForwardMaterial.h"
#include "ComputeMaterialFactory.h"
#include "../Maths/Vector.h"
#include "FrameBufferDiction.h"
#include <random>
#include <gli\gli.hpp>

// FIXME: Refactor
static uint32_t PLANET_COUNT = 4;
static uint32_t GROUP_SIZE = 16;

bool GlobalTextures::Init(const std::shared_ptr<GlobalTextures>& pSelf)
{
	if (!SelfRefBase<GlobalTextures>::Init(pSelf))
		return false;

	InitTextureDiction();
	InitScreenSizeTextureDiction();
	InitIBLTextures();
	InitSSAORandomRotationTexture();
	InitTransmittanceTextureDiction();
	InitSkyboxGenParameters();

	return true;
}

// FIXME: Make it configurable future
void GlobalTextures::InitTextureDiction()
{
	m_textureDiction.resize(InGameTextureTypeCount);

	m_textureDiction[RGBA8_1024].textureArrayName = "RGBA8TextureArray";
	m_textureDiction[RGBA8_1024].textureArrayDescription = "RGBA8, size16, mipLevel11";
	m_textureDiction[RGBA8_1024].pTextureArray = Image::CreateEmptyTexture2DArray(GetDevice(), { 1024, 1024 }, (uint32_t)std::log2(1024) + 1, 16, FrameBufferDiction::OFFSCREEN_COLOR_FORMAT);
	m_textureDiction[RGBA8_1024].maxSlotIndex = 0;
	m_textureDiction[RGBA8_1024].currentEmptySlot = 0;

	m_textureDiction[R8_1024].textureArrayName = "R8TextureArray";
	m_textureDiction[R8_1024].textureArrayDescription = "R8, size16, mipLevel11";
	m_textureDiction[R8_1024].pTextureArray = Image::CreateEmptyTexture2DArray(GetDevice(), { 1024, 1024 }, (uint32_t)std::log2(1024) + 1, 16, FrameBufferDiction::OFFSCREEN_SINGLE_COLOR_FORMAT);
	m_textureDiction[R8_1024].maxSlotIndex = 0;
	m_textureDiction[R8_1024].currentEmptySlot = 0;
}

void GlobalTextures::InitScreenSizeTextureDiction()
{
	m_textureDiction.resize(InGameTextureTypeCount);

	m_screenSizeTextureDiction.textureArrayName = "RGBA16ScreenSizeTextureArray";
	m_screenSizeTextureDiction.textureArrayDescription = "Mostly used to store intermedia data of current frames";

	Vector2d size = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();
	m_screenSizeTextureDiction.pTextureArray = Image::CreateMipmapOffscreenTexture2DArray(GetDevice(), { (uint32_t)size.x, (uint32_t)size.y }, 16, FrameBufferDiction::OFFSCREEN_COLOR_FORMAT);
	m_screenSizeTextureDiction.maxSlotIndex = 0;
	m_screenSizeTextureDiction.currentEmptySlot = 0;
}

void GlobalTextures::InitIBLTextures()
{
	m_IBL2DTextures.resize(IBL2DTextureTypeCount);

	Vector2ui size = 
	{ 
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().y,
	};
	m_IBL2DTextures[RGBA16_512_BRDFLut] = Image::CreateEmptyTexture2D(GetDevice(), size, FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT);

	for (uint32_t i = 0; i < IBLCubeTextureTypeCount; i++)
	{
		switch ((IBLTextureType)i)
		{
		case RGBA16_1024_SkyBox:
			for (uint32_t j = 0; j < 2; j++)
				m_IBLCubeTextures1[RGBA16_1024_SkyBox].push_back
				(
					Image::CreateEmptyCubeTexture(
						GetDevice(),
						{ ENV_MAP_SIZE, ENV_MAP_SIZE },
						1,
						FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT,
						VK_IMAGE_LAYOUT_GENERAL,
						VK_IMAGE_USAGE_STORAGE_BIT)
				);
			break;
		case RGBA16_512_SkyBoxIrradiance:
			for (uint32_t j = 0; j < 2; j++)
				m_IBLCubeTextures1[RGBA16_512_SkyBoxIrradiance].push_back
				(
					Image::CreateEmptyCubeTexture(
						GetDevice(),
						{ ENV_MAP_SIZE, ENV_MAP_SIZE },
						1,
						FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT,
						VK_IMAGE_LAYOUT_GENERAL,
						VK_IMAGE_USAGE_STORAGE_BIT)
				);
			break;

		case RGBA16_512_SkyBoxPrefilterEnv:
			for (uint32_t j = 0; j < 2; j++)
				m_IBLCubeTextures1[RGBA16_512_SkyBoxPrefilterEnv].push_back
				(
					Image::CreateEmptyCubeTexture(
						GetDevice(),
						{ ENV_MAP_SIZE, ENV_MAP_SIZE },
						(uint32_t)std::log2(512) + 1,
						FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT,
						VK_IMAGE_LAYOUT_GENERAL,
						VK_IMAGE_USAGE_STORAGE_BIT)
				);
			break;
		default:
			ASSERTION(false);
			break;
		}
	}
}

void GlobalTextures::InitSkyboxGenParameters()
{
	enum CubeCorner
	{
		BOTTOM_LEFT_FRONT,
		BOTTOM_RIGHT_FRONT,
		TOP_LEFT_FRONT,
		TOP_RIGHT_FRONT,
		BOTTOM_LEFT_BACK,
		BOTTOM_RIGHT_BACK,
		TOP_LEFT_BACK,
		TOP_RIGHT_BACK,
		CUBE_CORNER_COUNT
	};

	enum CubeFace
	{
		RIGHT,
		LEFT,
		TOP,
		BOTTOM,
		BACK,
		FRONT,
		CUBE_FACE_COUNT
	};

	Vector4f cubeCorners[] =
	{
		{ -1, -1,  1, 0 },
		{  1, -1,  1, 0 },
		{ -1,  1,  1, 0 },
		{  1,  1,  1, 0 },
		{ -1, -1, -1, 0 },
		{  1, -1, -1, 0 },
		{ -1,  1, -1, 0 },
		{  1,  1, -1, 0 }
	};

	m_cubeFaces[RIGHT][0] = cubeCorners[BOTTOM_RIGHT_BACK];
	m_cubeFaces[RIGHT][1] = cubeCorners[BOTTOM_RIGHT_FRONT];
	m_cubeFaces[RIGHT][2] = cubeCorners[TOP_RIGHT_BACK];
	m_cubeFaces[RIGHT][3] = cubeCorners[TOP_RIGHT_FRONT];

	m_cubeFaces[LEFT][0] = cubeCorners[BOTTOM_LEFT_FRONT];
	m_cubeFaces[LEFT][1] = cubeCorners[BOTTOM_LEFT_BACK];
	m_cubeFaces[LEFT][2] = cubeCorners[TOP_LEFT_FRONT];
	m_cubeFaces[LEFT][3] = cubeCorners[TOP_LEFT_BACK];

	m_cubeFaces[TOP][0] = cubeCorners[TOP_LEFT_BACK];
	m_cubeFaces[TOP][1] = cubeCorners[TOP_RIGHT_BACK];
	m_cubeFaces[TOP][2] = cubeCorners[TOP_LEFT_FRONT];
	m_cubeFaces[TOP][3] = cubeCorners[TOP_RIGHT_FRONT];

	m_cubeFaces[BOTTOM][0] = cubeCorners[BOTTOM_LEFT_FRONT];
	m_cubeFaces[BOTTOM][1] = cubeCorners[BOTTOM_RIGHT_FRONT];
	m_cubeFaces[BOTTOM][2] = cubeCorners[BOTTOM_LEFT_BACK];
	m_cubeFaces[BOTTOM][3] = cubeCorners[BOTTOM_RIGHT_BACK];

	// Note: our coordinate system is right-hand based, however, cube map is left-hand based
	// So here BACK = positive z and FRONT = negative z
	m_cubeFaces[BACK][0] = cubeCorners[BOTTOM_LEFT_BACK];
	m_cubeFaces[BACK][1] = cubeCorners[BOTTOM_RIGHT_BACK];
	m_cubeFaces[BACK][2] = cubeCorners[TOP_LEFT_BACK];
	m_cubeFaces[BACK][3] = cubeCorners[TOP_RIGHT_BACK];

	m_cubeFaces[FRONT][0] = cubeCorners[BOTTOM_RIGHT_FRONT];
	m_cubeFaces[FRONT][1] = cubeCorners[BOTTOM_LEFT_FRONT];
	m_cubeFaces[FRONT][2] = cubeCorners[TOP_RIGHT_FRONT];
	m_cubeFaces[FRONT][3] = cubeCorners[TOP_LEFT_FRONT];

	for (uint32_t i = 0; i < (uint32_t)CubeFace::CUBE_FACE_COUNT; i++)
		for (uint32_t j = 0; j < 4; j++)
			m_cubeFaces[i][j].Normalize();

	m_envJobCounter = 0;
	m_envTexturePingpongIndex = 0;
	m_envGenState = EnvGenState::SKYBOX_GEN;
	m_lastEnvGenState = EnvGenState::SKYBOX_GEN;
}

void GlobalTextures::GenerateSkyBox(uint32_t chunkIndex)
{
	if (m_pIBLGenCmdBuffer != nullptr && m_lastEnvGenState != EnvGenState::WAITING_FOR_COMPLETE)
	{
		FrameWorkManager::GetInstance()->SubmitCommandBuffers
		(
			GlobalObjects()->GetComputeQueue(), 
			{ m_pIBLGenCmdBuffer }, 
			{}, 
			{}, 
			{},
			false, false
		);
	}

	GlobalObjects()->GetThreadTaskQueue()->AddJobA(
	[this, chunkIndex](const std::shared_ptr<PerFrameResource>& pPerFrameRes)
	{
		if (m_envGenState != EnvGenState::WAITING_FOR_COMPLETE)
		{
			m_pIBLGenCmdBuffer = pPerFrameRes->AllocateTransientComputeCommandBuffer();
			m_pIBLGenCmdBuffer->StartPrimaryRecording();
		}

		if (m_envGenState == EnvGenState::SKYBOX_GEN)
		{
			m_lastEnvGenState = m_envGenState;
			// Record world space camera and main ligh direction
			// This info remains unchanged until next round of skybox gen
			if (m_envJobCounter == 0)
			{
				m_wsCameraPosition = UniformData::GetInstance()->GetPerFrameUniforms()->GetCameraPosition().SinglePrecision();
				m_wsMainLightDir = UniformData::GetInstance()->GetPerFrameUniforms()->GetWorldSpaceMainLightDir().SinglePrecision();
			}

			if (m_pSkyboxGenMaterial == nullptr)
			{
				m_pSkyboxGenMaterial = CreateSkyboxGenMaterial
				(
					m_IBLCubeTextures1[RGBA16_1024_SkyBox]
				);
			}

			m_cubeFaces[m_envJobCounter][0].w = (float)m_envJobCounter;
			m_cubeFaces[m_envJobCounter][1].w = (float)chunkIndex;
			m_cubeFaces[m_envJobCounter][3].w = (float)m_envTexturePingpongIndex;

			m_pSkyboxGenMaterial->UpdatePushConstantData(&m_cubeFaces[m_envJobCounter][0], 0, sizeof(m_cubeFaces[m_envJobCounter]));
			m_pSkyboxGenMaterial->UpdatePushConstantData
			(
				&m_wsCameraPosition,
				sizeof(m_cubeFaces[m_envJobCounter]), 
				sizeof(m_wsCameraPosition)
			);
			m_pSkyboxGenMaterial->UpdatePushConstantData
			(
				&m_wsMainLightDir,
				sizeof(m_cubeFaces[m_envJobCounter]) + sizeof(Vector4f),
				sizeof(m_wsMainLightDir)
			);
			m_pSkyboxGenMaterial->BeforeRenderPass(m_pIBLGenCmdBuffer);
			m_pSkyboxGenMaterial->Dispatch(m_pIBLGenCmdBuffer);
			m_pSkyboxGenMaterial->AfterRenderPass(m_pIBLGenCmdBuffer);

			m_envJobCounter++;

			if (m_envJobCounter == 6)
			{
				m_envGenState = EnvGenState::IRRADIANCE_GEN;
				m_envJobCounter = 0;
			}
		}
		else if (m_envGenState == EnvGenState::IRRADIANCE_GEN)
		{
			m_lastEnvGenState = m_envGenState;

			static uint32_t groupCountOneDispatchBorder = 8;
			static uint32_t groupCountOneDispatch = groupCountOneDispatchBorder * groupCountOneDispatchBorder;
			static uint32_t dispatchCountPerBorder = ENV_MAP_SIZE / GROUP_SIZE / groupCountOneDispatchBorder;
			static uint32_t dispatchCountPerFace = dispatchCountPerBorder * dispatchCountPerBorder;

			uint32_t faceID = m_envJobCounter / dispatchCountPerFace;
			uint32_t groupOffset = m_envJobCounter % dispatchCountPerFace;
			uint32_t groupOffsetX = groupOffset % dispatchCountPerBorder;
			uint32_t groupOffsetY = groupOffset / dispatchCountPerBorder;

			if (m_pIrradianceGenMaterial == nullptr)
			{
				m_pIrradianceGenMaterial = CreateIrradianceGenMaterial
				(
					m_IBLCubeTextures1[RGBA16_1024_SkyBox],
					m_IBLCubeTextures1[RGBA16_512_SkyBoxIrradiance]
				);
			}

			m_cubeFaces[faceID][0].w = (float)faceID;
			m_cubeFaces[faceID][1].w = (float)groupOffsetX * groupCountOneDispatchBorder * GROUP_SIZE;
			m_cubeFaces[faceID][2].w = (float)groupOffsetY * groupCountOneDispatchBorder * GROUP_SIZE;
			m_cubeFaces[faceID][3].w = (float)m_envTexturePingpongIndex;
			m_pIrradianceGenMaterial->UpdatePushConstantData(&m_cubeFaces[faceID][0], 0, sizeof(m_cubeFaces[faceID]));
			m_pIrradianceGenMaterial->BeforeRenderPass(m_pIBLGenCmdBuffer);
			m_pIrradianceGenMaterial->Dispatch(m_pIBLGenCmdBuffer);
			m_pIrradianceGenMaterial->AfterRenderPass(m_pIBLGenCmdBuffer);

			m_envJobCounter++;

			if (m_envJobCounter == dispatchCountPerFace * 6)
			{
				m_envGenState = EnvGenState::REFLECTION_GEN;
				m_envJobCounter = 0;
			}
		}
		else if (m_envGenState == EnvGenState::REFLECTION_GEN)
		{
			m_lastEnvGenState = m_envGenState;

			static uint32_t mipLevels = (uint32_t)std::log2((double)ENV_MAP_SIZE) + 1;
			for (uint32_t i = 0; i < mipLevels; i++)
			{
				if (i == m_reflectionGenMaterials.size())
				{
					m_reflectionGenMaterials.push_back
					(
						CreateReflectionGenMaterial
						(
							m_IBLCubeTextures1[RGBA16_1024_SkyBox],
							m_IBLCubeTextures1[RGBA16_512_SkyBoxPrefilterEnv],
							i
						)
					);
				}

				std::shared_ptr<Material> pMaterial = m_reflectionGenMaterials[i];

				m_cubeFaces[m_envJobCounter][0].w = (float)m_envJobCounter;
				m_cubeFaces[m_envJobCounter][1].w = i / (float)(mipLevels - 1);	// Roughness
				m_cubeFaces[m_envJobCounter][3].w = (float)m_envTexturePingpongIndex;
				pMaterial->UpdatePushConstantData(&m_cubeFaces[m_envJobCounter][0], 0, sizeof(m_cubeFaces[m_envJobCounter]));
				pMaterial->BeforeRenderPass(m_pIBLGenCmdBuffer);
				pMaterial->Dispatch(m_pIBLGenCmdBuffer);
				pMaterial->AfterRenderPass(m_pIBLGenCmdBuffer);
			}

			m_envJobCounter++;

			if (m_envJobCounter == 6)
			{
				m_envGenState = EnvGenState::WAITING_FOR_COMPLETE;
				m_envJobCounter = 0;

				std::vector<VkImageMemoryBarrier> queueReleaseBarrier(IBLCubeTextureTypeCount);
				for (uint32_t i = 0; i < IBLCubeTextureTypeCount; i++)
				{
					queueReleaseBarrier[i] = {};
					queueReleaseBarrier[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					queueReleaseBarrier[i].image = m_IBLCubeTextures1[i][m_envTexturePingpongIndex]->GetDeviceHandle();

					queueReleaseBarrier[i].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
					queueReleaseBarrier[i].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
					queueReleaseBarrier[i].srcQueueFamilyIndex = GlobalObjects()->GetComputeQueue()->GetQueueFamilyIndex();

					queueReleaseBarrier[i].dstAccessMask = 0;
					queueReleaseBarrier[i].newLayout = VK_IMAGE_LAYOUT_GENERAL;
					queueReleaseBarrier[i].dstQueueFamilyIndex = GlobalObjects()->GetGraphicQueue()->GetQueueFamilyIndex();

					queueReleaseBarrier[i].subresourceRange =
					{
						VK_IMAGE_ASPECT_COLOR_BIT,
						0, m_IBLCubeTextures1[i][m_envTexturePingpongIndex]->GetImageInfo().mipLevels,
						0, m_IBLCubeTextures1[i][m_envTexturePingpongIndex]->GetImageInfo().arrayLayers
					};
				}

				m_pIBLGenCmdBuffer->AttachBarriers
				(
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					{},
					{},
					queueReleaseBarrier
				);
			}
		}
		// Wait for another circle to ensure last batch of generating work is done
		// This is ensured by mechanism of FrameManager
		// FIXME: However, I think I should create another mechanism to handle all of this kind of async resource preparation
		else if (m_envGenState == EnvGenState::WAITING_FOR_COMPLETE)
		{
			m_lastEnvGenState = m_envGenState;

			m_envJobCounter++;
			if (m_envJobCounter == GetSwapChain()->GetSwapChainImageCount())
			{
				m_lastEnvGenState = m_envGenState;
				m_envGenState = EnvGenState::SKYBOX_GEN;
				m_envJobCounter = 0;

				std::shared_ptr<CommandBuffer> pCmd = pPerFrameRes->AllocateTransientPrimaryCommandBuffer();
				pCmd->StartPrimaryRecording();
				std::vector<VkImageMemoryBarrier> queueAcquireBarrier(IBLCubeTextureTypeCount);
				for (uint32_t i = 0; i < IBLCubeTextureTypeCount; i++)
				{
					queueAcquireBarrier[i] = {};
					queueAcquireBarrier[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					queueAcquireBarrier[i].image = m_IBLCubeTextures1[i][m_envTexturePingpongIndex]->GetDeviceHandle();

					queueAcquireBarrier[i].srcAccessMask = 0;
					queueAcquireBarrier[i].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
					queueAcquireBarrier[i].srcQueueFamilyIndex = GlobalObjects()->GetComputeQueue()->GetQueueFamilyIndex();

					queueAcquireBarrier[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					queueAcquireBarrier[i].newLayout = VK_IMAGE_LAYOUT_GENERAL;
					queueAcquireBarrier[i].dstQueueFamilyIndex = GlobalObjects()->GetGraphicQueue()->GetQueueFamilyIndex();

					queueAcquireBarrier[i].subresourceRange =
					{
						VK_IMAGE_ASPECT_COLOR_BIT,
						0, m_IBLCubeTextures1[i][m_envTexturePingpongIndex]->GetImageInfo().mipLevels,
						0, m_IBLCubeTextures1[i][m_envTexturePingpongIndex]->GetImageInfo().arrayLayers
					};
				}

				pCmd->AttachBarriers
				(
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					{},
					{},
					queueAcquireBarrier
				);
				pCmd->EndPrimaryRecording();

				FrameWorkManager::GetInstance()->SubmitCommandBuffers
				(
					GlobalObjects()->GetGraphicQueue(), 
					{ pCmd }, 
					{}, 
					{ VK_PIPELINE_STAGE_ALL_COMMANDS_BIT }, 
					{}, 
					false, false
				);

				// Set ping pong index of the completed
				UniformData::GetInstance()->GetPerFrameUniforms()->SetEnvPingpongIndex(m_envTexturePingpongIndex);

				// Start next ping pong
				m_envTexturePingpongIndex = (m_envTexturePingpongIndex + 1) % 2;
			}
		}

		if (m_lastEnvGenState != EnvGenState::WAITING_FOR_COMPLETE)
			m_pIBLGenCmdBuffer->EndPrimaryRecording();
	}, FrameWorkManager::GetInstance()->FrameIndex());
}

void GlobalTextures::InitSSAORandomRotationTexture()
{
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
	std::default_random_engine randomEngine;

	std::vector<Vector4f> tangents;
	for (uint32_t i = 0; i < SSAO_RANDOM_ROTATION_COUNT; i++)
	{
		Vector3f tangent = { randomFloats(randomEngine) * 2.0f - 1.0f, randomFloats(randomEngine) * 2.0f - 1.0f, 0 };
		tangent.Normalize();
		tangents.push_back({ tangent, 0 });	// NOTE: make it 4 units to pair with gpu variable alignment
	}

	gli::texture2d tex = gli::texture2d(gli::FORMAT_RGBA32_SFLOAT_PACK32, { std::sqrt(SSAO_RANDOM_ROTATION_COUNT), std::sqrt(SSAO_RANDOM_ROTATION_COUNT) }, 1);
	std::memcpy(tex.data(), tangents.data(), tex.size());

	m_pSSAORandomRotations = Image::CreateTexture2D(GetDevice(), { {tex} }, VK_FORMAT_R32G32B32A32_SFLOAT);
}

void GlobalTextures::GenerateBRDFLUTTexture()
{
	SceneGenerator::GetInstance()->GenerateBRDFLUTGenScene();

	RenderWorkManager::GetInstance()->SetRenderStateMask(RenderWorkManager::BrdfLutGen);

	std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadGraphicPool()->AllocatePrimaryCommandBuffer();

	std::vector<VkClearValue> clearValues =
	{
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0 }
	};

	VkViewport viewport =
	{
		0, 0,
		(float)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x, (float)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().y,
		0, 1
	};

	VkRect2D scissorRect =
	{
		0, 0,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().y,
	};

	SceneGenerator::GetInstance()->GetRootObject()->Update();
	SceneGenerator::GetInstance()->GetRootObject()->LateUpdate();
	SceneGenerator::GetInstance()->GetRootObject()->UpdateCachedData();
	SceneGenerator::GetInstance()->GetRootObject()->OnPreRender();
	SceneGenerator::GetInstance()->GetRootObject()->OnRenderObject();
	SceneGenerator::GetInstance()->GetRootObject()->OnPostRender();
	UniformData::GetInstance()->SyncDataBuffer();
	SceneGenerator::GetInstance()->GetMaterial0()->SyncBufferData();

	SceneGenerator::GetInstance()->GetMaterial0()->OnFrameBegin();
	pDrawCmdBuffer->StartPrimaryRecording();

	SceneGenerator::GetInstance()->GetMaterial0()->BeforeRenderPass(pDrawCmdBuffer);
	RenderPassDiction::GetInstance()->GetForwardRenderPassOffScreen()->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_EnvGenOffScreen)[0]);
	SceneGenerator::GetInstance()->GetMaterial0()->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_EnvGenOffScreen));
	RenderPassDiction::GetInstance()->GetForwardRenderPassOffScreen()->EndRenderPass(pDrawCmdBuffer);
	SceneGenerator::GetInstance()->GetMaterial0()->AfterRenderPass(pDrawCmdBuffer);

	pDrawCmdBuffer->EndPrimaryRecording();
	SceneGenerator::GetInstance()->GetMaterial0()->OnFrameEnd();

	GlobalGraphicQueue()->SubmitCommandBuffer(pDrawCmdBuffer, nullptr, true);

	FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_EnvGenOffScreen)[0]->ExtractContent(m_IBL2DTextures[RGBA16_512_BRDFLut]);
}

void GlobalTextures::InitTransmittanceTextureDiction()
{
	// FIXME: Size of these textures are hard-coded for now
	for (uint32_t i = 0; i < PLANET_COUNT; i++)
	{
		m_transmittanceTextureDiction.push_back(Image::CreateEmptyTexture2DForCompute(GetDevice(), { 256, 64 }, VK_FORMAT_R32G32B32A32_SFLOAT));
		m_scatterTextureDiction.push_back(Image::CreateEmptyTexture3D(GetDevice(), { 256, 128, 32 }, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL));
		m_irradianceTextureDiction.push_back(Image::CreateEmptyTexture2DForCompute(GetDevice(), { 64, 16 }, VK_FORMAT_R32G32B32A32_SFLOAT));
		m_pDeltaIrradiance = Image::CreateEmptyTexture2DForCompute(GetDevice(), { 64, 16 }, VK_FORMAT_R32G32B32A32_SFLOAT);
		m_pDeltaRayleigh = Image::CreateEmptyTexture3D(GetDevice(), { 256, 128, 32 }, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL);
		m_pDeltaMie = Image::CreateEmptyTexture3D(GetDevice(), { 256, 128, 32 }, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL);
		m_pDeltaScatterDensity = Image::CreateEmptyTexture3D(GetDevice(), { 256, 128, 32 }, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL);
		m_pDeltaMultiScatter = Image::CreateEmptyTexture3D(GetDevice(), { 256, 128, 32 }, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL);
	}
}

std::shared_ptr<GlobalTextures> GlobalTextures::Create()
{
	std::shared_ptr<GlobalTextures> pGlobalTextures = std::make_shared<GlobalTextures>();
	if (pGlobalTextures.get() && pGlobalTextures->Init(pGlobalTextures))
		return pGlobalTextures;
	return nullptr;
}

std::vector<UniformVarList> GlobalTextures::PrepareUniformVarList() const
{
	return 
	{
		{
			CombinedSampler,
			"RGBA8_1024_Mip_Texture_Array"
		},
		{
			CombinedSampler,
			"R8_1024_Mip_Texture_Array"
		},
		{
			CombinedSampler,
			"RGBA16_Screen_Size_Mip_Texture_Array"
		},
		{
			CombinedSampler,
			"R8_512_Texture_2D_BRDFLUT"
		},
		{
			CombinedSampler,
			"RGBA32_4_SSAO_Random_Rotation"
		},
		{
			CombinedSampler,
			"RGBA32 w:256, h:64, transmittance texture diction",
			{},
			PLANET_COUNT
		},
		{
			CombinedSampler,
			"RGBA32 w:256, h:128, d:32, single scatter texture diction",
			{},
			PLANET_COUNT
		},
		{
			CombinedSampler,
			"RGBA32 w:64, h:16, indirect irradiance texture diction",
			{},
			PLANET_COUNT
		},
		// FIXME: Temporary binding here, just for debugging in profile tool
		{
			CombinedSampler,
			"RGBA32 w:64, h:16, delta irradiance",
			{},
		},
		{
			CombinedSampler,
			"RGBA32 w:256, h:128, d:32, delta rayleigh",
			{},
		},
		{
			CombinedSampler,
			"RGBA32 w:256, h:128, d:32, delta mie",
			{},
		},
		{
			CombinedSampler,
			"RGBA32 w:256, h:128, d:32, delta scatter density",
			{},
		},
		{
			CombinedSampler,
			"RGBA32 w:256, h:128, d:32, delta multi scatter",
			{},
		},
		{
			CombinedSampler,
			"Runtime generated skybox cube texture",
			{},
			2
		},
		{
			CombinedSampler,
			"Runtime generated irradiance cube texture",
			{},
			2
		},
		{
			CombinedSampler,
			"Runtime generated reflection cube texture",
			{},
			2
		}
	};
}

void GlobalTextures::InsertScreenSizeTexture(const TextureDesc& desc)
{
	uint32_t emptySlot;
	InsertTextureDesc(desc, m_screenSizeTextureDiction, emptySlot);
}

void GlobalTextures::InsertTextureDesc(const TextureDesc& desc, TextureArrayDesc& textureArr, uint32_t& emptySlot)
{
	if (textureArr.lookupTable.find(desc.textureName) != textureArr.lookupTable.end())
		return;

	emptySlot = textureArr.currentEmptySlot;
	textureArr.textureDescriptions[emptySlot] = desc;
	textureArr.lookupTable[desc.textureName] = emptySlot;	// Record lookup table

	// Find if there's an available slot within the pool
	bool found = false;
	for (uint32_t i = 0; i < textureArr.maxSlotIndex; i++)
	{
		if (textureArr.textureDescriptions.find(i) == textureArr.textureDescriptions.end())
		{
			textureArr.currentEmptySlot = i;
			found = true;
			break;
		}
	}

	// If there's no available slot within, then increase slot count by 1 and assign empty slot to it
	if (!found)
	{
		textureArr.currentEmptySlot = textureArr.maxSlotIndex + 1;
		textureArr.maxSlotIndex = textureArr.currentEmptySlot;
	}
}

void GlobalTextures::InsertTexture(InGameTextureType type, const TextureDesc& desc, const gli::texture2d& gliTexture2d)
{
	uint32_t emptySlot;
	InsertTextureDesc(desc, m_textureDiction[type], emptySlot);
	m_textureDiction[type].pTextureArray->InsertTexture(gliTexture2d, emptySlot);
}

bool GlobalTextures::GetTextureIndex(const TextureArrayDesc& textureArr, const std::string& textureName, uint32_t& textureIndex)
{
	auto it = textureArr.lookupTable.find(textureName);
	if (it == textureArr.lookupTable.end())
		return false;

	textureIndex = it->second;
	return true;
}

bool GlobalTextures::GetTextureIndex(InGameTextureType type, const std::string& textureName, uint32_t& textureIndex)
{
	return GetTextureIndex(m_textureDiction[type], textureName, textureIndex);
}

bool GlobalTextures::GetScreenSizeTextureIndex(const std::string& textureName, uint32_t& textureIndex)
{
	return GetTextureIndex(m_screenSizeTextureDiction, textureName, textureIndex);
}

uint32_t GlobalTextures::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	// Bind global texture array
	for (uint32_t i = 0; i < InGameTextureTypeCount; i++)
	{
		std::shared_ptr<Image> pTexArray = GetTextureArray((InGameTextureType)i);
		pDescriptorSet->UpdateImage(bindingIndex++, pTexArray, pTexArray->CreateLinearRepeatSampler(), pTexArray->CreateDefaultImageView());
	}

	pDescriptorSet->UpdateImage(bindingIndex++, m_screenSizeTextureDiction.pTextureArray, m_screenSizeTextureDiction.pTextureArray->CreateLinearRepeatSampler(), m_screenSizeTextureDiction.pTextureArray->CreateDefaultImageView());

	// Binding global IBL texture2d
	for (uint32_t i = 0; i < IBL2DTextureTypeCount; i++)
	{
		std::shared_ptr<Image> pTexture2D = GetIBLTexture2D((IBLTextureType)i);
		pDescriptorSet->UpdateImage(bindingIndex++, pTexture2D, pTexture2D->CreateLinearClampToEdgeSampler(), pTexture2D->CreateDefaultImageView());
	}

	pDescriptorSet->UpdateImage(bindingIndex++, m_pSSAORandomRotations, m_pSSAORandomRotations->CreateNearestRepeatSampler(), m_pSSAORandomRotations->CreateDefaultImageView());

	// Binding atmosphere precomputed textures
	// 1. Transmittance
	std::vector<CombinedImage> imgs;
	for (uint32_t i = 0; i < PLANET_COUNT; i++)
	{
		imgs.push_back({ m_transmittanceTextureDiction[i], m_transmittanceTextureDiction[i]->CreateLinearClampToEdgeSampler(), m_transmittanceTextureDiction[i]->CreateDefaultImageView() });
	}
	pDescriptorSet->UpdateImages(bindingIndex++, imgs);

	// 2. Scatter
	imgs.clear();
	for (uint32_t i = 0; i < PLANET_COUNT; i++)
	{
		imgs.push_back({ m_scatterTextureDiction[i], m_scatterTextureDiction[i]->CreateLinearClampToEdgeSampler(), m_scatterTextureDiction[i]->CreateDefaultImageView() });
	}
	pDescriptorSet->UpdateImages(bindingIndex++, imgs);

	// 3. Irradiance
	imgs.clear();
	for (uint32_t i = 0; i < PLANET_COUNT; i++)
	{
		imgs.push_back({ m_irradianceTextureDiction[i], m_irradianceTextureDiction[i]->CreateLinearClampToEdgeSampler(), m_irradianceTextureDiction[i]->CreateDefaultImageView() });
	}
	pDescriptorSet->UpdateImages(bindingIndex++, imgs);

	// FIXME: Temporary binding here, just for debugging in profile tool
	pDescriptorSet->UpdateImage(bindingIndex++, { m_pDeltaIrradiance, m_pDeltaIrradiance->CreateLinearClampToEdgeSampler(), m_pDeltaIrradiance->CreateDefaultImageView() });
	pDescriptorSet->UpdateImage(bindingIndex++, { m_pDeltaRayleigh, m_pDeltaRayleigh->CreateLinearClampToEdgeSampler(), m_pDeltaRayleigh->CreateDefaultImageView() });
	pDescriptorSet->UpdateImage(bindingIndex++, { m_pDeltaMie, m_pDeltaMie->CreateLinearClampToEdgeSampler(), m_pDeltaMie->CreateDefaultImageView() });
	pDescriptorSet->UpdateImage(bindingIndex++, { m_pDeltaScatterDensity, m_pDeltaScatterDensity->CreateLinearClampToEdgeSampler(), m_pDeltaScatterDensity->CreateDefaultImageView() });
	pDescriptorSet->UpdateImage(bindingIndex++, { m_pDeltaMultiScatter, m_pDeltaMultiScatter->CreateLinearClampToEdgeSampler(), m_pDeltaMultiScatter->CreateDefaultImageView() });

	std::vector<CombinedImage> skybox;
	std::vector<CombinedImage> irradiance;
	std::vector<CombinedImage> reflection;

	for (uint32_t i = 0; i < 2; i++)
	{
		skybox.push_back
		(
			{
				m_IBLCubeTextures1[RGBA16_1024_SkyBox][i],
				m_IBLCubeTextures1[RGBA16_1024_SkyBox][i]->CreateLinearClampToEdgeSampler(),
				m_IBLCubeTextures1[RGBA16_1024_SkyBox][i]->CreateDefaultImageView()
			}
		);
	}

	for (uint32_t i = 0; i < 2; i++)
	{
		irradiance.push_back
		(
			{
				m_IBLCubeTextures1[RGBA16_512_SkyBoxIrradiance][i],
				m_IBLCubeTextures1[RGBA16_512_SkyBoxIrradiance][i]->CreateLinearClampToEdgeSampler(),
				m_IBLCubeTextures1[RGBA16_512_SkyBoxIrradiance][i]->CreateDefaultImageView()
			}
		);
	}

	for (uint32_t i = 0; i < 2; i++)
	{
		reflection.push_back
		(
			{
				m_IBLCubeTextures1[RGBA16_512_SkyBoxPrefilterEnv][i],
				m_IBLCubeTextures1[RGBA16_512_SkyBoxPrefilterEnv][i]->CreateLinearClampToEdgeSampler(),
				m_IBLCubeTextures1[RGBA16_512_SkyBoxPrefilterEnv][i]->CreateDefaultImageView()
			}
		);
	}

	pDescriptorSet->UpdateImages(bindingIndex++, skybox);
	pDescriptorSet->UpdateImages(bindingIndex++, irradiance);
	pDescriptorSet->UpdateImages(bindingIndex++, reflection);

	return bindingIndex;
}

