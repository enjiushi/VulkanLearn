#include "RenderWorkManager.h"
#include "ComputeMaterialFactory.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/Image.h"
#include "../vulkan/GlobalVulkanStates.h"
#include "../common/Util.h"
#include "RenderPassDiction.h"
#include "ForwardRenderPass.h"
#include "DeferredMaterial.h"
#include "ShadowMapMaterial.h"
#include "ForwardMaterial.h"
#include "PostProcessingMaterial.h"
#include "GBufferPlanetMaterial.h"
#include "MaterialInstance.h"

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

		case MotionTileMax:		
		{
			std::vector<std::shared_ptr<Image>> inputImages;
			std::vector<std::shared_ptr<Image>> outputImages;
			for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
			{
				inputImages.push_back(FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_GBuffer)[i]->GetColorTarget(FrameBufferDiction::MotionVector));
				outputImages.push_back(FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_MotionTileMax)[i]->GetColorTarget(0));
			}
			m_materials[i] = { { CreateTileMaxMaterial(inputImages, outputImages) } };
		}break;
		case MotionNeighborMax:
		{
			std::vector<std::shared_ptr<Image>> inputImages;
			std::vector<std::shared_ptr<Image>> outputImages;
			for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
			{
				inputImages.push_back(FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_MotionTileMax)[i]->GetColorTarget(0));
				outputImages.push_back(FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_MotionNeighborMax)[i]->GetColorTarget(0));
			}
			m_materials[i] = { { CreateNeighborMaxMaterial(inputImages, outputImages) } };
		}break;
		case Shadow:			m_materials[i] = { { ShadowMapMaterial::CreateDefaultMaterial() } }; break;
		case SkinnedShadow:		m_materials[i] = { { ShadowMapMaterial::CreateDefaultMaterial(true) } }; break;
		case SSAOSSR:			m_materials[i] = { { CreateSSAOSSRMaterial() } }; break;
		case SSAOBlurV:			
		{
			std::vector<std::shared_ptr<Image>> inputImages;
			std::vector<std::shared_ptr<Image>> outputImages;
			for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
			{
				inputImages.push_back(FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_SSAOSSR)[i]->GetColorTarget(0));
				outputImages.push_back(FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_SSAOBlurV)[i]->GetColorTarget(0));
			}
			m_materials[i] = { { CreateGaussianBlurMaterial(inputImages, outputImages, { true, 1, 1 }) } };
		}break;
		case SSAOBlurH:			
		{
			std::vector<std::shared_ptr<Image>> inputImages;
			std::vector<std::shared_ptr<Image>> outputImages;
			for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
			{
				inputImages.push_back(FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_SSAOBlurV)[i]->GetColorTarget(0));
				outputImages.push_back(FrameBufferDiction::GetInstance()->GetFrameBuffers(FrameBufferDiction::FrameBufferType_SSAOBlurH)[i]->GetColorTarget(0));
			}
			m_materials[i] = { { CreateGaussianBlurMaterial(inputImages, outputImages,{ false, 1, 1 }) } };
		}break;
		case DeferredShading:	m_materials[i] = { { CreateDeferredShadingMaterial() } }; break;
		case TemporalResolve:	m_materials[i] = { { CreateTemporalResolveMaterial(0), CreateTemporalResolveMaterial(1) } }; break;
		case DepthOfField:
		{
			for (uint32_t j = 0; j < (uint32_t)DOFPass::COUNT; j++)
			{
				m_materials[i].materialSet.push_back(CreateDOFMaterial((DOFPass)j));
			}
		}break;
		case BloomDownSample:
		{
			for (uint32_t j = 0; j < BLOOM_ITER_COUNT; j++)
			{
				BloomPass bloomPass = (j == 0) ? BloomPass::PREFILTER : BloomPass::DOWNSAMPLE;
				m_materials[i].materialSet.push_back(CreateBloomMaterial(bloomPass, j));
			}
		}break;
		case BloomUpSample:
		{
			for (uint32_t j = 0; j < BLOOM_ITER_COUNT; j++)
			{
				m_materials[i].materialSet.push_back(CreateBloomMaterial(BloomPass::UPSAMPLE, j));
			}
		}break;
		case Combine:			m_materials[i] = { { CreateCombineMaterial() } }; break;
		case PostProcess:		m_materials[i] = { { PostProcessingMaterial::CreateDefaultMaterial() } }; break;
		case SkyboxGen:			m_materials[i] = { { CreateSkyboxGenMaterial(UniformData::GetInstance()->GetGlobalTextures()->GetIBLTextureCube1(RGBA16_1024_SkyBox)) } }; break;
		case IrradianceGen1:	m_materials[i] = { { CreateIrradianceGenMaterial(UniformData::GetInstance()->GetGlobalTextures()->GetIBLTextureCube1(RGBA16_1024_SkyBox), UniformData::GetInstance()->GetGlobalTextures()->GetIBLTextureCube1(RGBA16_512_SkyBoxIrradiance)) } }; break;
						 
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
	GetMaterial(MotionTileMax)->Dispatch(pDrawCmdBuffer, pingpong);
	GetMaterial(MotionTileMax)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(MotionNeighborMax)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(MotionNeighborMax)->Dispatch(pDrawCmdBuffer, pingpong);
	GetMaterial(MotionNeighborMax)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(Shadow)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(SkinnedShadow)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShadowMap)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_ShadowMap));
	GetMaterial(Shadow)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_ShadowMap), pingpong);
	GetMaterial(SkinnedShadow)->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_ShadowMap), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShadowMap)->EndRenderPass(pDrawCmdBuffer);
	GetMaterial(SkinnedShadow)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(Shadow)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(SSAOSSR)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(SSAOSSR)->Dispatch(pDrawCmdBuffer, pingpong);
	GetMaterial(SSAOSSR)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(SSAOBlurV)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(SSAOBlurV)->Dispatch(pDrawCmdBuffer, pingpong);
	GetMaterial(SSAOBlurV)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(SSAOBlurH)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(SSAOBlurH)->Dispatch(pDrawCmdBuffer, pingpong);
	GetMaterial(SSAOBlurH)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(DeferredShading)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(DeferredShading)->Dispatch(pDrawCmdBuffer, pingpong);
	GetMaterial(DeferredShading)->AfterRenderPass(pDrawCmdBuffer, pingpong);


	GetMaterial(TemporalResolve, pingpong)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(TemporalResolve, pingpong)->Dispatch(pDrawCmdBuffer, pingpong);
	GetMaterial(TemporalResolve, pingpong)->AfterRenderPass(pDrawCmdBuffer, pingpong);

	for (uint32_t i = 0; i < (uint32_t)DOFPass::COUNT; i++)
	{
		GetMaterial(DepthOfField, i)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
		GetMaterial(DepthOfField, i)->Dispatch(pDrawCmdBuffer, pingpong);
		GetMaterial(DepthOfField, i)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	}

	// Downsample first
	for (uint32_t i = 0; i < BLOOM_ITER_COUNT; i++)
	{
		GetMaterial(BloomDownSample, i)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
		GetMaterial(BloomDownSample, i)->Dispatch(pDrawCmdBuffer, pingpong);
		GetMaterial(BloomDownSample, i)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	}

	// Upsample then
	for (int32_t i = BLOOM_ITER_COUNT - 1; i >= 0; i--)
	{
		GetMaterial(BloomUpSample, i)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
		GetMaterial(BloomUpSample, i)->Dispatch(pDrawCmdBuffer, pingpong);
		GetMaterial(BloomUpSample, i)->AfterRenderPass(pDrawCmdBuffer, pingpong);
	}

	GetMaterial(Combine)->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	GetMaterial(Combine)->Dispatch(pDrawCmdBuffer, pingpong);
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