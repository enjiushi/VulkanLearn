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
#include "../class/RenderWorkManager.h"
#include "../class/Mesh.h"
#include "../component/MeshRenderer.h"
#include "../Base/BaseObject.h"
#include "../scene/SceneGenerator.h"
#include "../class/RenderPassDiction.h"
#include "../vulkan/DescriptorSet.h"
#include "ForwardRenderPass.h"
#include "GlobalTextures.h"
#include "Material.h"
#include "ForwardMaterial.h"
#include "../Maths/Vector.h"
#include "FrameBufferDiction.h"
#include <random>
#include <gli\gli.hpp>

// FIXME: Refactor
static uint32_t PLANET_COUNT = 4;

bool GlobalTextures::Init(const std::shared_ptr<GlobalTextures>& pSelf)
{
	if (!SelfRefBase<GlobalTextures>::Init(pSelf))
		return false;

	InitTextureDiction();
	InitScreenSizeTextureDiction();
	InitIBLTextures();
	InitSSAORandomRotationTexture();
	InitTransmittanceTextureDiction();

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
	m_IBLCubeTextures.resize(IBLCubeTextureTypeCount);
	m_IBLCubeTextures[RGBA16_1024_SkyBox] = Image::CreateEmptyCubeTexture(GetDevice(), { 1024, 1024 }, (uint32_t)std::log2(1024) + 1, FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT);
	m_IBLCubeTextures[RGBA16_512_SkyBoxIrradiance] = Image::CreateEmptyCubeTexture(GetDevice(), { (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().y }, 1, FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT);
	m_IBLCubeTextures[RGBA16_512_SkyBoxPrefilterEnv] = Image::CreateEmptyCubeTexture(GetDevice(), { (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().y }, (uint32_t)std::log2(512) + 1, FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT);

	m_IBL2DTextures.resize(IBL2DTextureTypeCount);

	Vector2ui size = 
	{ 
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().y,
	};
	m_IBL2DTextures[RGBA16_512_BRDFLut] = Image::CreateEmptyTexture2D(GetDevice(), size, FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT);
}

void GlobalTextures::InitIBLTextures(const gli::texture_cube& skyBoxTex)
{
	m_IBLCubeTextures[RGBA16_1024_SkyBox]->UpdateByteStream({ {skyBoxTex} });
	InitIrradianceTexture();
	InitPrefilterEnvTexture();
	InitBRDFLUTTexture();
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

void GlobalTextures::InitIrradianceTexture()
{
	SceneGenerator::GetInstance()->GenerateIrradianceGenScene();

	RenderWorkManager::GetInstance()->SetRenderStateMask(RenderWorkManager::IrradianceGen);

	Vector3d up = { 0, 1, 0 };
	Vector3d look = { 0, 0, -1 };
	look.Normalize();
	Vector3d xaxis = up ^ look.Negativate();
	xaxis.Normalize();
	Vector3d yaxis = look ^ xaxis;
	yaxis.Normalize();

	Matrix3d rotation;
	rotation.c[0] = xaxis;
	rotation.c[1] = yaxis;
	rotation.c[2] = look;

	Matrix3d cameraRotations[] =
	{
		Matrix3d::Rotation(1.0 * 3.14159265 / 1.0, Vector3d(1, 0, 0)) * Matrix3d::Rotation(3.0 * 3.14159265 / 2.0, Vector3d(0, 1, 0)) * rotation,	// Positive X, i.e right
		Matrix3d::Rotation(1.0 * 3.14159265 / 1.0, Vector3d(1, 0, 0)) * Matrix3d::Rotation(1.0 * 3.14159265 / 2.0, Vector3d(0, 1, 0)) * rotation,	// Negative X, i.e left
		Matrix3d::Rotation(3.0 * 3.14159265 / 2.0, Vector3d(1, 0, 0)) * rotation,	// Positive Y, i.e top
		Matrix3d::Rotation(1.0 * 3.14159265 / 2.0, Vector3d(1, 0, 0)) * rotation,	// Negative Y, i.e bottom
		Matrix3d::Rotation(1.0 * 3.14159265 / 1.0, Vector3d(1, 0, 0)) * rotation,	// Positive Z, i.e back
		Matrix3d::Rotation(1.0 * 3.14159265 / 1.0, Vector3d(0, 0, 1)) * rotation,	// Negative Z, i.e front
	};

	for (uint32_t i = 0; i < 6; i++)
	{
		std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadGraphicPool()->AllocatePrimaryCommandBuffer();

		std::vector<VkClearValue> clearValues =
		{
			{ 0.0, 0.0, 0.0, 0.0 },
			{ 1.0, 0 }
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

		SceneGenerator::GetInstance()->GetCameraObject()->SetRotation(cameraRotations[i]);

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
		SceneGenerator::GetInstance()->GetMaterial0()->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_EnvGenOffScreen));
		RenderPassDiction::GetInstance()->GetForwardRenderPassOffScreen()->EndRenderPass(pDrawCmdBuffer);
		SceneGenerator::GetInstance()->GetMaterial0()->AfterRenderPass(pDrawCmdBuffer);

		pDrawCmdBuffer->EndPrimaryRecording();
		SceneGenerator::GetInstance()->GetMaterial0()->OnFrameEnd();

		GlobalGraphicQueue()->SubmitCommandBuffer(pDrawCmdBuffer, nullptr, true);

		FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_EnvGenOffScreen)[0]->ExtractContent(m_IBLCubeTextures[RGBA16_512_SkyBoxIrradiance], 0, 1, i, 1);
	}
}

void GlobalTextures::InitPrefilterEnvTexture()
{
	SceneGenerator::GetInstance()->GeneratePrefilterEnvGenScene();

	RenderWorkManager::GetInstance()->SetRenderStateMask(RenderWorkManager::ReflectionGen);

	Vector3d up = { 0, 1, 0 };
	Vector3d look = { 0, 0, -1 };
	look.Normalize();
	Vector3d xaxis = up ^ look.Negativate();
	xaxis.Normalize();
	Vector3d yaxis = look ^ xaxis;
	yaxis.Normalize();

	Matrix3d rotation;
	rotation.c[0] = xaxis;
	rotation.c[1] = yaxis;
	rotation.c[2] = look;

	Matrix3d cameraRotations[] =
	{
		Matrix3d::Rotation(1.0 * 3.14159265 / 1.0, Vector3d(1, 0, 0)) * Matrix3d::Rotation(3.0 * 3.14159265 / 2.0, Vector3d(0, 1, 0)) * rotation,	// Positive X, i.e right
		Matrix3d::Rotation(1.0 * 3.14159265 / 1.0, Vector3d(1, 0, 0)) * Matrix3d::Rotation(1.0 * 3.14159265 / 2.0, Vector3d(0, 1, 0)) * rotation,	// Negative X, i.e left
		Matrix3d::Rotation(3.0 * 3.14159265 / 2.0, Vector3d(1, 0, 0)) * rotation,	// Positive Y, i.e top
		Matrix3d::Rotation(1.0 * 3.14159265 / 2.0, Vector3d(1, 0, 0)) * rotation,	// Negative Y, i.e bottom
		Matrix3d::Rotation(1.0 * 3.14159265 / 1.0, Vector3d(1, 0, 0)) * rotation,	// Positive Z, i.e back
		Matrix3d::Rotation(1.0 * 3.14159265 / 1.0, Vector3d(0, 0, 1)) * rotation,	// Negative Z, i.e front
	};

	uint32_t mipLevels = (uint32_t)std::log2(UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x);
	for (uint32_t mipLevel = 0; mipLevel < mipLevels + 1; mipLevel++)
	{
		UniformData::GetInstance()->GetPerFrameUniforms()->SetPadding0(mipLevel / (float)mipLevels);
		uint32_t size = (uint32_t)std::pow(2, mipLevels - mipLevel);
		for (uint32_t i = 0; i < 6; i++)
		{
			std::shared_ptr<CommandBuffer> pDrawCmdBuffer = MainThreadGraphicPool()->AllocatePrimaryCommandBuffer();

			std::vector<VkClearValue> clearValues =
			{
				{ 0.0, 0.0, 0.0, 0.0 },
				{ 1.0, 0 }
			};

			VkViewport viewport =
			{
				0, 0,
				(float)size, (float)size,
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
			SceneGenerator::GetInstance()->GetMaterial0()->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_EnvGenOffScreen), 0, true);
			RenderPassDiction::GetInstance()->GetForwardRenderPassOffScreen()->EndRenderPass(pDrawCmdBuffer);
			SceneGenerator::GetInstance()->GetMaterial0()->AfterRenderPass(pDrawCmdBuffer);

			pDrawCmdBuffer->EndPrimaryRecording();
			SceneGenerator::GetInstance()->GetMaterial0()->OnFrameEnd();

			GlobalGraphicQueue()->SubmitCommandBuffer(pDrawCmdBuffer, nullptr, true);

			FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_EnvGenOffScreen)[0]->ExtractContent(m_IBLCubeTextures[RGBA16_512_SkyBoxPrefilterEnv], mipLevel, 1, i, 1, size, size);
		}
	}
}

void GlobalTextures::InitBRDFLUTTexture()
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
		m_pDeltaIrradiance = Image::CreateEmptyTexture2DForCompute(GetDevice(), { 64, 16 }, VK_FORMAT_R32G32B32A32_SFLOAT);
		m_pDeltaRayleigh = Image::CreateEmptyTexture3D(GetDevice(), { 256, 128, 32 }, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL);
		m_pDeltaMie = Image::CreateEmptyTexture3D(GetDevice(), { 256, 128, 32 }, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL);
		m_pDeltaScatterDensity = Image::CreateEmptyTexture3D(GetDevice(), { 256, 128, 32 }, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_GENERAL);
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

	// Binding global IBL texture cube
	for (uint32_t i = 0; i < IBLCubeTextureTypeCount; i++)
	{
		std::shared_ptr<Image> pTextureCube = GetIBLTextureCube((IBLTextureType)i);
		pDescriptorSet->UpdateImage(bindingIndex++, pTextureCube, pTextureCube->CreateLinearRepeatSampler(), pTextureCube->CreateDefaultImageView());
	}

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

	// 2. Single Scatter
	imgs.clear();
	for (uint32_t i = 0; i < PLANET_COUNT; i++)
	{
		imgs.push_back({ m_scatterTextureDiction[i], m_scatterTextureDiction[i]->CreateLinearClampToEdgeSampler(), m_scatterTextureDiction[i]->CreateDefaultImageView() });
	}
	pDescriptorSet->UpdateImages(bindingIndex++, imgs);

	// FIXME: Temporary binding here, just for debugging in profile tool
	pDescriptorSet->UpdateImage(bindingIndex++, { m_pDeltaIrradiance, m_pDeltaIrradiance->CreateLinearClampToEdgeSampler(), m_pDeltaIrradiance->CreateDefaultImageView() });
	pDescriptorSet->UpdateImage(bindingIndex++, { m_pDeltaRayleigh, m_pDeltaRayleigh->CreateLinearClampToEdgeSampler(), m_pDeltaRayleigh->CreateDefaultImageView() });
	pDescriptorSet->UpdateImage(bindingIndex++, { m_pDeltaMie, m_pDeltaMie->CreateLinearClampToEdgeSampler(), m_pDeltaMie->CreateDefaultImageView() });
	pDescriptorSet->UpdateImage(bindingIndex++, { m_pDeltaScatterDensity, m_pDeltaScatterDensity->CreateLinearClampToEdgeSampler(), m_pDeltaScatterDensity->CreateDefaultImageView() });
	return bindingIndex;
}

