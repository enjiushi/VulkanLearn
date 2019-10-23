#include "RenderWorkManager.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/DepthStencilBuffer.h"
#include "../vulkan/Texture2D.h"
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
#include "MaterialInstance.h"

bool RenderWorkManager::Init()
{
	if (!Singleton<RenderWorkManager>::Init())
		return false;

	m_PBRGbufferMaterial			= GBufferMaterial::CreateDefaultMaterial();
	m_PBRSkinnedGbufferMaterial		= GBufferMaterial::CreateDefaultMaterial(true);
	m_pMotionTileMaxMaterial		= MotionTileMaxMaterial::CreateDefaultMaterial();
	m_pMotionNeighborMaxMaterial	= MotionNeighborMaxMaterial::CreateDefaultMaterial();
	m_pShadingMaterial				= DeferredShadingMaterial::CreateDefaultMaterial();
	m_pTemporalResolveMaterials.push_back(TemporalResolveMaterial::CreateDefaultMaterial(0));
	m_pTemporalResolveMaterials.push_back(TemporalResolveMaterial::CreateDefaultMaterial(1));
	for (uint32_t i = 0; i < DOFMaterial::DOFPass_Count; i++)
	{
		m_DOFMaterials.push_back(DOFMaterial::CreateDefaultMaterial((DOFMaterial::DOFPass)i));
	}
	m_pShadowMapMaterial			= ShadowMapMaterial::CreateDefaultMaterial();
	m_pSkinnedShadowMapMaterial		= ShadowMapMaterial::CreateDefaultMaterial(true);
	m_pSSAOMaterial					= SSAOMaterial::CreateDefaultMaterial();
	for (uint32_t i = 0; i < BLOOM_ITER_COUNT; i++)
	{
		BloomMaterial::BloomPass bloomPass = (i == 0) ? BloomMaterial::BloomPass_Prefilter : BloomMaterial::BloomPass_DownSampleBox13;

		m_bloomDownsampleMaterials.push_back(BloomMaterial::CreateDefaultMaterial(bloomPass, i));
		m_bloomUpsampleMaterials.push_back(BloomMaterial::CreateDefaultMaterial(BloomMaterial::BloomPass_UpSampleTent, i));
	}
	m_pSSAOBlurVMaterial			= GaussianBlurMaterial::CreateDefaultMaterial(FrameBufferDiction::FrameBufferType_SSAOSSR, FrameBufferDiction::FrameBufferType_SSAOBlurV, RenderPassDiction::PipelineRenderPassSSAOBlurV, { true, 1, 1 });
	m_pSSAOBlurHMaterial			= GaussianBlurMaterial::CreateDefaultMaterial(FrameBufferDiction::FrameBufferType_SSAOBlurV, FrameBufferDiction::FrameBufferType_SSAOBlurH, RenderPassDiction::PipelineRenderPassSSAOBlurH, { false, 1, 1 });
	m_pCombineMaterial				= CombineMaterial::CreateDefaultMaterial();
	m_pPostProcessMaterial			= PostProcessingMaterial::CreateDefaultMaterial();

	SimpleMaterialCreateInfo info = {};
	info.shaderPaths = { L"../data/shaders/sky_box.vert.spv", L"", L"", L"", L"../data/shaders/sky_box.frag.spv", L"" };
	info.materialUniformVars = {};
	info.vertexFormat = VertexFormatP;
	info.vertexFormatInMem = VertexFormatP;
	info.subpassIndex = 1;
	info.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading);
	info.depthWriteEnable = false;
	info.frameBufferType = FrameBufferDiction::FrameBufferType_Shading;

	m_pSkyBoxMaterial = ForwardMaterial::CreateDefaultMaterial(info);

	info = {};
	info.shaderPaths = { L"../data/shaders/background_motion_gen.vert.spv", L"", L"", L"", L"../data/shaders/background_motion_gen.frag.spv", L"" };
	info.materialUniformVars = {};
	info.vertexFormat = 0;
	info.vertexFormatInMem = 0;
	info.subpassIndex = 1;
	info.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer);
	info.depthWriteEnable = false;
	info.frameBufferType = FrameBufferDiction::FrameBufferType_GBuffer;

	m_pBackgroundMotionMaterial = ForwardMaterial::CreateDefaultMaterial(info);

	return true;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquirePBRMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = m_PBRGbufferMaterial->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << Scene);
	return pMaterialInstance;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquirePBRSkinnedMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = m_PBRSkinnedGbufferMaterial->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << Scene);
	return pMaterialInstance;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquireShadowMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = m_pShadowMapMaterial->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << ShadowMapGen);
	return pMaterialInstance;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquireSkinnedShadowMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = m_pSkinnedShadowMapMaterial->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << ShadowMapGen);
	return pMaterialInstance;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquireSkyBoxMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = m_pSkyBoxMaterial->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << Scene);
	return pMaterialInstance;
}

void RenderWorkManager::SyncMaterialData()
{
	m_PBRGbufferMaterial->SyncBufferData();
	m_PBRSkinnedGbufferMaterial->SyncBufferData();
	m_pShadowMapMaterial->SyncBufferData();
	m_pSkinnedShadowMapMaterial->SyncBufferData();
}

void RenderWorkManager::Draw(const std::shared_ptr<CommandBuffer>& pDrawCmdBuffer, uint32_t pingpong)
{
	m_PBRGbufferMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	m_PBRSkinnedGbufferMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	m_pBackgroundMotionMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer));
	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y });
	m_PBRGbufferMaterial->DrawIndirect(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer), pingpong);
	m_PBRSkinnedGbufferMaterial->DrawIndirect(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->NextSubpass(pDrawCmdBuffer);
	m_pBackgroundMotionMaterial->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer));
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->EndRenderPass(pDrawCmdBuffer);
	m_pBackgroundMotionMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);
	m_PBRSkinnedGbufferMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);
	m_PBRGbufferMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);


	m_pMotionTileMaxMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionTileMax)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_MotionTileMax));
	m_pMotionTileMaxMaterial->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_MotionTileMax), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionTileMax)->EndRenderPass(pDrawCmdBuffer);
	m_pMotionTileMaxMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);


	m_pMotionNeighborMaxMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionNeighborMax)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_MotionNeighborMax));
	m_pMotionNeighborMaxMaterial->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_MotionNeighborMax), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionNeighborMax)->EndRenderPass(pDrawCmdBuffer);
	m_pMotionNeighborMaxMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);


	m_pShadowMapMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	m_pSkinnedShadowMapMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShadowMap)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_ShadowMap));
	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetShadowGenWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetShadowGenWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetShadowGenWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetShadowGenWindowSize().y });
	m_pShadowMapMaterial->DrawIndirect(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_ShadowMap), pingpong);
	m_pSkinnedShadowMapMaterial->DrawIndirect(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_ShadowMap), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShadowMap)->EndRenderPass(pDrawCmdBuffer);
	m_pSkinnedShadowMapMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);
	m_pShadowMapMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);


	m_pSSAOMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOSSR)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOSSR));
	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().y });
	m_pSSAOMaterial->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOSSR), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOSSR)->EndRenderPass(pDrawCmdBuffer);
	m_pSSAOMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);


	m_pSSAOBlurVMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurV)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurV));
	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().y });
	m_pSSAOBlurVMaterial->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurV), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurV)->EndRenderPass(pDrawCmdBuffer);
	m_pSSAOBlurVMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);


	m_pSSAOBlurHMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurH)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurH));
	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().y });
	m_pSSAOBlurHMaterial->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurH), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurH)->EndRenderPass(pDrawCmdBuffer);
	m_pSSAOBlurHMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);


	m_pShadingMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	m_pSkyBoxMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Shading));
	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y });
	m_pShadingMaterial->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Shading), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->NextSubpass(pDrawCmdBuffer);
	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y });
	m_pSkyBoxMaterial->DrawIndirect(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Shading), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->EndRenderPass(pDrawCmdBuffer);
	m_pSkyBoxMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);
	m_pShadingMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);


	m_pTemporalResolveMaterials[pingpong]->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassTemporalResolve)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, (FrameMgr()->FrameIndex() + 1) % GetSwapChain()->GetSwapChainImageCount(), (pingpong + 1) % 2));
	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y });
	m_pTemporalResolveMaterials[pingpong]->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetPingPongFrameBuffer(FrameBufferDiction::FrameBufferType_TemporalResolve, (FrameMgr()->FrameIndex() + 1) % GetSwapChain()->GetSwapChainImageCount(), (pingpong + 1) % 2));
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassTemporalResolve)->EndRenderPass(pDrawCmdBuffer);
	m_pTemporalResolveMaterials[pingpong]->AfterRenderPass(pDrawCmdBuffer, pingpong);

	for (uint32_t i = 0; i < DOFMaterial::DOFPass_Count; i++)
	{
		std::shared_ptr<FrameBuffer> pTargetFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_DOF, i);
		Vector2ui size = { pTargetFrameBuffer->GetFramebufferInfo().width, pTargetFrameBuffer->GetFramebufferInfo().height };

		m_DOFMaterials[i]->BeforeRenderPass(pDrawCmdBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassDOF)->BeginRenderPass(pDrawCmdBuffer, pTargetFrameBuffer);
		GetGlobalVulkanStates()->SetViewport({ 0, 0, (float)size.x, (float)size.y, 0, 1 });
		GetGlobalVulkanStates()->SetScissorRect({ 0, 0, size.x, size.y });
		m_DOFMaterials[i]->DrawScreenQuad(pDrawCmdBuffer, pTargetFrameBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassDOF)->EndRenderPass(pDrawCmdBuffer);
		m_DOFMaterials[i]->AfterRenderPass(pDrawCmdBuffer, pingpong);
	}

	// Downsample first
	for (uint32_t i = 0; i < BLOOM_ITER_COUNT; i++)
	{
		std::shared_ptr<FrameBuffer> pTargetFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Bloom, i + 1);
		Vector2ui size = { pTargetFrameBuffer->GetFramebufferInfo().width, pTargetFrameBuffer->GetFramebufferInfo().height };

		m_bloomDownsampleMaterials[i]->BeforeRenderPass(pDrawCmdBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->BeginRenderPass(pDrawCmdBuffer, pTargetFrameBuffer);
		GetGlobalVulkanStates()->SetViewport({ 0, 0, (float)size.x, (float)size.y, 0, 1 });
		GetGlobalVulkanStates()->SetScissorRect({ 0, 0, size.x, size.y });
		m_bloomDownsampleMaterials[i]->DrawScreenQuad(pDrawCmdBuffer, pTargetFrameBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->EndRenderPass(pDrawCmdBuffer);
		m_bloomDownsampleMaterials[i]->AfterRenderPass(pDrawCmdBuffer, pingpong);
	}

	// Upsample then
	for (int32_t i = BLOOM_ITER_COUNT - 1; i >= 0; i--)
	{
		std::shared_ptr<FrameBuffer> pTargetFrameBuffer = FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Bloom, i);
		Vector2ui size = { pTargetFrameBuffer->GetFramebufferInfo().width, pTargetFrameBuffer->GetFramebufferInfo().height };

		m_bloomUpsampleMaterials[i]->BeforeRenderPass(pDrawCmdBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->BeginRenderPass(pDrawCmdBuffer, pTargetFrameBuffer);
		GetGlobalVulkanStates()->SetViewport({ 0, 0, (float)size.x, (float)size.y, 0, 1 });
		GetGlobalVulkanStates()->SetScissorRect({ 0, 0, size.x, size.y });
		m_bloomUpsampleMaterials[i]->DrawScreenQuad(pDrawCmdBuffer, pTargetFrameBuffer, pingpong);
		RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->EndRenderPass(pDrawCmdBuffer);
		m_bloomUpsampleMaterials[i]->AfterRenderPass(pDrawCmdBuffer, pingpong);
	}

	m_pCombineMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassCombine)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_CombineResult));
	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y });
	m_pCombineMaterial->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_CombineResult), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassCombine)->EndRenderPass(pDrawCmdBuffer);
	m_pCombineMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);


	m_pPostProcessMaterial->BeforeRenderPass(pDrawCmdBuffer, pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassPostProcessing)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_PostProcessing));
	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y });
	m_pPostProcessMaterial->DrawScreenQuad(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_PostProcessing), pingpong);
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassPostProcessing)->EndRenderPass(pDrawCmdBuffer);
	m_pPostProcessMaterial->AfterRenderPass(pDrawCmdBuffer, pingpong);
}

void RenderWorkManager::OnFrameBegin()
{
	m_PBRGbufferMaterial->OnFrameBegin();
	m_PBRSkinnedGbufferMaterial->OnFrameBegin();
	m_pShadowMapMaterial->OnFrameBegin();
	m_pSkinnedShadowMapMaterial->OnFrameBegin();
	m_pSkyBoxMaterial->OnFrameBegin();
}

void RenderWorkManager::OnFrameEnd()
{
	m_pSkyBoxMaterial->OnFrameEnd();
	m_pSkinnedShadowMapMaterial->OnFrameEnd();
	m_pShadowMapMaterial->OnFrameEnd();
	m_PBRSkinnedGbufferMaterial->OnFrameEnd();
	m_PBRGbufferMaterial->OnFrameEnd();
}