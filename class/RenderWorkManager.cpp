#include "RenderWorkManager.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/Image.h"
#include "../vulkan/GlobalVulkanStates.h"
#include "RenderPassDiction.h"
#include "ForwardRenderPass.h"
#include "DeferredMaterial.h"
#include "MotionTileMaxMaterial.h"
#include "MotionNeighborMaxMaterial.h"
#include "ShadowMapMaterial.h"
#include "SSAOMaterial.h"
#include "GaussianBlurMaterial.h"
#include "BloomMaterial.h"
#include "ForwardMaterial.h"
#include "TemporalResolveMaterial.h"
#include "CombineMaterial.h"
#include "PostProcessingMaterial.h"
#include "DOFMaterial.h"
#include "GBufferPlanetMaterial.h"
#include "MaterialInstance.h"

void RenderWorkManager::InitComputeMaterialVariables()
{
	// FIXME: hard-code
	static uint32_t groupSize = 16;
	Vector3ui groupNum =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x / 16,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y / 16,
		1
	};

	std::vector<std::shared_ptr<Image>> DOFResults;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pDOFResult = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_DOF, FrameBufferDiction::CombineLayer)[j];
		DOFResults.push_back(pDOFResult->GetColorTarget(0));
	}

	std::vector<std::shared_ptr<Image>> bloomTextures;
	for (uint32_t j = 0; j < GetSwapChain()->GetSwapChainImageCount(); j++)
	{
		std::shared_ptr<FrameBuffer> pBloomFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_Bloom)[j];

		bloomTextures.push_back(pBloomFrameBuffer->GetColorTarget(0));
	}

	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	textureUnits.push_back
	(
		{
			3,

			DOFResults,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				// Only a pre-barrier is needed
				{
					true,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_IMAGE_LAYOUT_GENERAL,
					VK_ACCESS_SHADER_READ_BIT
				},
				{
					false
				}
			}
		}
	);

	textureUnits.push_back
	(
		{
			4,

			bloomTextures,
			VK_IMAGE_ASPECT_COLOR_BIT,
			{ 0, 1, 0, 1 },
			false,

			CustomizedComputeMaterial::TextureUnit::BY_FRAME,

			{
				// Only a pre-barrier is needed
				{
					true,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,

					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_IMAGE_LAYOUT_GENERAL,
					VK_ACCESS_SHADER_READ_BIT
				},
				{
					false
				}
			}
		}
	);

	m_computeMaterialVariables[Combine] = 
	{
		L"../data/shaders/combine.comp.spv",
		groupNum,
		textureUnits,
		{}
	};
}

bool RenderWorkManager::Init()
{
	if (!Singleton<RenderWorkManager>::Init())
		return false;

	m_materials.resize(MaterialEnumCount);
	for (uint32_t i = 0; i < MaterialEnumCount; i++)
	{
		switch ((MaterialEnum)i)
		{
		case PBRGBuffer:		m_materials[i] = { { GBufferMaterial::CreateDefaultMaterial()} }; break;
		case PBRSkinnedGBuffer: m_materials[i] = { { GBufferMaterial::CreateDefaultMaterial(true) } }; break;
		case PBRPlanetGBuffer:	m_materials[i] = { { GBufferPlanetMaterial::CreateDefaultMaterial() } }; break;
		case BackgroundMotion:	
		{
			SimpleMaterialCreateInfo info = {};
			info = {};
			info.shaderPaths = { L"../data/shaders/background_motion_gen.vert.spv", L"", L"", L"", L"../data/shaders/background_motion_gen.frag.spv", L"" };
			info.materialUniformVars = {};
			info.vertexFormat = 0;
			info.vertexFormatInMem = 0;
			info.subpassIndex = 1;
			info.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer);
			info.depthWriteEnable = false;
			info.frameBufferType = FrameBufferDiction::FrameBufferType_GBuffer;

			m_materials[i] = { {ForwardMaterial::CreateDefaultMaterial(info)} };
		}break;

		case MotionTileMax:		m_materials[i] = { { MotionTileMaxMaterial::CreateDefaultMaterial() } }; break;
		case MotionNeighborMax:	m_materials[i] = { { MotionNeighborMaxMaterial::CreateDefaultMaterial() } }; break;
		case Shadow:			m_materials[i] = { { ShadowMapMaterial::CreateDefaultMaterial() } }; break;
		case SkinnedShadow:		m_materials[i] = { { ShadowMapMaterial::CreateDefaultMaterial(true) } }; break;
		case SSAO:				m_materials[i] = { { SSAOMaterial::CreateDefaultMaterial() } }; break;
		case SSAOBlurV:			m_materials[i] = { { GaussianBlurMaterial::CreateDefaultMaterial(FrameBufferDiction::FrameBufferType_SSAOSSR, FrameBufferDiction::FrameBufferType_SSAOBlurV, RenderPassDiction::PipelineRenderPassSSAOBlurV,{ true, 1, 1 }) } }; break;
		case SSAOBlurH:			m_materials[i] = { { GaussianBlurMaterial::CreateDefaultMaterial(FrameBufferDiction::FrameBufferType_SSAOBlurV, FrameBufferDiction::FrameBufferType_SSAOBlurH, RenderPassDiction::PipelineRenderPassSSAOBlurH,{ false, 1, 1 }) } }; break;
		case DeferredShading:	m_materials[i] = { { DeferredShadingMaterial::CreateDefaultMaterial() } }; break;
		case SkyBox:
		{
			SimpleMaterialCreateInfo info = {};
			info.shaderPaths = { L"../data/shaders/sky_box.vert.spv", L"", L"", L"", L"../data/shaders/sky_box.frag.spv", L"" };
			info.materialUniformVars = {};
			info.vertexFormat = VertexFormatP;
			info.vertexFormatInMem = VertexFormatP;
			info.subpassIndex = 1;
			info.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading);
			info.depthWriteEnable = false;
			info.frameBufferType = FrameBufferDiction::FrameBufferType_Shading;

			m_materials[i] = { { ForwardMaterial::CreateDefaultMaterial(info) } };
		} break;
		case TemporalResolve:	m_materials[i] = { { TemporalResolveMaterial::CreateDefaultMaterial(0), TemporalResolveMaterial::CreateDefaultMaterial(1) } }; break;
		case DepthOfField:
		{
			for (uint32_t j = 0; j < DOFMaterial::DOFPass_Count; j++)
			{
				m_materials[i].materialSet.push_back(DOFMaterial::CreateDefaultMaterial((DOFMaterial::DOFPass)j));
			}
		}break;
		case BloomDownSample:
		{
			for (uint32_t j = 0; j < BLOOM_ITER_COUNT; j++)
			{
				BloomMaterial::BloomPass bloomPass = (j == 0) ? BloomMaterial::BloomPass_Prefilter : BloomMaterial::BloomPass_DownSampleBox13;
				m_materials[i].materialSet.push_back(BloomMaterial::CreateDefaultMaterial(bloomPass, j));
			}
		}break;
		case BloomUpSample:
		{
			for (uint32_t j = 0; j < BLOOM_ITER_COUNT; j++)
			{
				BloomMaterial::BloomPass bloomPass = (j == 0) ? BloomMaterial::BloomPass_Prefilter : BloomMaterial::BloomPass_DownSampleBox13;
				m_materials[i].materialSet.push_back(BloomMaterial::CreateDefaultMaterial(BloomMaterial::BloomPass_UpSampleTent, j));
			}
		}break;
		case Combine:			m_materials[i] = { { CombineMaterial::CreateDefaultMaterial() } }; break;
		case PostProcess:		m_materials[i] = { { PostProcessingMaterial::CreateDefaultMaterial() } }; break;
						 
		default:
			ASSERTION(false);
			break;
		}
	}

	return true;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquirePBRMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = GetMaterial(PBRGBuffer)->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << Scene);
	return pMaterialInstance;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquirePBRSkinnedMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = GetMaterial(PBRSkinnedGBuffer)->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << Scene);
	return pMaterialInstance;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquirePBRPlanetMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = GetMaterial(PBRPlanetGBuffer)->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << Scene);
	return pMaterialInstance;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquireShadowMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = GetMaterial(Shadow)->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << ShadowMapGen);
	return pMaterialInstance;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquireSkinnedShadowMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = GetMaterial(SkinnedShadow)->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << ShadowMapGen);
	return pMaterialInstance;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquireSkyBoxMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = GetMaterial(SkyBox)->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << Scene);
	return pMaterialInstance;
}

void RenderWorkManager::SyncMaterialData()
{
	for (auto& materialSet : m_materials)
	{
		for (auto pMaterial : materialSet.materialSet)
		{
			pMaterial->SyncBufferData();
		}
	}
}

void RenderWorkManager::Draw(const std::shared_ptr<CommandBuffer>& pDrawCmdBuffer, uint32_t pingpong)
{
	GetMaterial(PBRGBuffer)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(PBRSkinnedGBuffer)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(PBRPlanetGBuffer)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(BackgroundMotion)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer));
	GetMaterial(PBRGBuffer)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer), pingpong);
	GetMaterial(PBRSkinnedGBuffer)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer), pingpong);
	GetMaterial(PBRPlanetGBuffer)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->NextSubpass(pDrawCmdBuffer);
	GetMaterial(BackgroundMotion)->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer));
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(BackgroundMotion)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(PBRPlanetGBuffer)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(PBRSkinnedGBuffer)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(PBRGBuffer)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(MotionTileMax)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionTileMax)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_MotionTileMax));
	GetMaterial(MotionTileMax)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_MotionTileMax), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionTileMax)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(MotionTileMax)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(MotionNeighborMax)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionNeighborMax)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_MotionNeighborMax));
	GetMaterial(MotionNeighborMax)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_MotionNeighborMax), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionNeighborMax)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(MotionNeighborMax)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(Shadow)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(SkinnedShadow)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShadowMap)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_ShadowMap));
	GetMaterial(Shadow)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_ShadowMap), pingpong);
	GetMaterial(SkinnedShadow)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_ShadowMap), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShadowMap)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(SkinnedShadow)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(Shadow)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(SSAO)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOSSR)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOSSR));
	GetMaterial(SSAO)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOSSR), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOSSR)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(SSAO)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(SSAOBlurV)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurV)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurV));
	GetMaterial(SSAOBlurV)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurV), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurV)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(SSAOBlurV)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(SSAOBlurH)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurH)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurH));
	GetMaterial(SSAOBlurH)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurH), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurH)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(SSAOBlurH)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(DeferredShading)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(SkyBox)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Shading));
	GetMaterial(DeferredShading)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Shading), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->NextSubpass(pDrawCmdBuffer);
	GetMaterial(SkyBox)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Shading), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(SkyBox)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(DeferredShading)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(TemporalResolve, pingpong)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassTemporalResolve)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, (FrameMgr()->FrameIndex() + 1) % GetSwapChain()->GetSwapChainImageCount(), (pingpong + 1) % 2));
	GetMaterial(TemporalResolve, pingpong)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, (FrameMgr()->FrameIndex() + 1) % GetSwapChain()->GetSwapChainImageCount(), (pingpong + 1) % 2));
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassTemporalResolve)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(TemporalResolve, pingpong)->AfterRenderPass(pDrawCmdBuffer, pingpong);

	for (uint32_t i = 0; i < DOFMaterial::DOFPass_Count; i++)
	{
		std::shared_ptr<FrameBuffer> pTargetFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_DOF, i);
		Vector2ui size = { pTargetFrameBuffer->GetFramebufferInfo().width, pTargetFrameBuffer->GetFramebufferInfo().height };

		GetMaterial(DepthOfField, i)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassDOF)->BeginRenderPass(pDrawCmdBuffer, pTargetFrameBuffer);
		GetMaterial(DepthOfField, i)->Draw(pDrawCmdBuffer, pTargetFrameBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassDOF)->EndRenderPass(pDrawCmdBuffer);
		GetMaterial(DepthOfField, i)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	}

	// Downsample first
	for (uint32_t i = 0; i < BLOOM_ITER_COUNT; i++)
	{
		std::shared_ptr<FrameBuffer> pTargetFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Bloom, i + 1);
		Vector2ui size = { pTargetFrameBuffer->GetFramebufferInfo().width, pTargetFrameBuffer->GetFramebufferInfo().height };

		GetMaterial(BloomDownSample, i)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->BeginRenderPass(pDrawCmdBuffer, pTargetFrameBuffer);
		GetMaterial(BloomDownSample, i)->Draw(pDrawCmdBuffer, pTargetFrameBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->EndRenderPass(pDrawCmdBuffer);
		GetMaterial(BloomDownSample, i)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	}

	// Upsample then
	for (int32_t i = BLOOM_ITER_COUNT - 1; i >= 0; i--)
	{
		std::shared_ptr<FrameBuffer> pTargetFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Bloom, i);
		Vector2ui size = { pTargetFrameBuffer->GetFramebufferInfo().width, pTargetFrameBuffer->GetFramebufferInfo().height };

		GetMaterial(BloomUpSample, i)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->BeginRenderPass(pDrawCmdBuffer, pTargetFrameBuffer);
		GetMaterial(BloomUpSample, i)->Draw(pDrawCmdBuffer, pTargetFrameBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->EndRenderPass(pDrawCmdBuffer);
		GetMaterial(BloomUpSample, i)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	}

	GetMaterial(Combine)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassCombine)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_CombineResult));
	GetMaterial(Combine)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_CombineResult), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassCombine)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(Combine)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(PostProcess)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassPostProcessing)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_PostProcessing));
	GetMaterial(PostProcess)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_PostProcessing), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassPostProcessing)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(PostProcess)->AfterRenderPass(pDrawCmdBuffer, pingpong);
}

void RenderWorkManager::OnFrameBegin()
{
	for (auto& materialSet : m_materials)
	{
		for (auto pMaterial : materialSet.materialSet)
		{
			pMaterial->OnFrameBegin();
		}
	}
}

void RenderWorkManager::OnFrameEnd()
{
	for (auto& materialSet : m_materials)
	{
		for (auto pMaterial : materialSet.materialSet)
		{
			pMaterial->OnFrameEnd();
		}
	}
}