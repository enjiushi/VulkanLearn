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
#include "ShadowMapMaterial.h"
#include "SSAOMaterial.h"
#include "GaussianBlurMaterial.h"
#include "BloomMaterial.h"
#include "ForwardMaterial.h"
#include "PostProcessingMaterial.h"
#include "MaterialInstance.h"

bool RenderWorkManager::Init()
{
	if (!Singleton<RenderWorkManager>::Init())
		return false;

	m_PBRGbufferMaterial = GBufferMaterial::CreateDefaultMaterial();
	m_pShadingMaterial = DeferredShadingMaterial::CreateDefaultMaterial();
	m_pShadowMapMaterial = ShadowMapMaterial::CreateDefaultMaterial();
	m_pSSAOMaterial = SSAOMaterial::CreateDefaultMaterial();
	m_pBloomMaterial = BloomMaterial::CreateDefaultMaterial();
	m_pSSAOBlurVMaterial = GaussianBlurMaterial::CreateDefaultMaterial(FrameBufferDiction::FrameBufferType_SSAO, FrameBufferDiction::FrameBufferType_SSAOBlurV, RenderPassDiction::PipelineRenderPassSSAOBlurV, { true, 1, 1 });
	m_pSSAOBlurHMaterial = GaussianBlurMaterial::CreateDefaultMaterial(FrameBufferDiction::FrameBufferType_SSAOBlurV, FrameBufferDiction::FrameBufferType_SSAOBlurH, RenderPassDiction::PipelineRenderPassSSAOBlurH, { false, 1, 1 });
	m_pBloomBlurVMaterial = GaussianBlurMaterial::CreateDefaultMaterial(FrameBufferDiction::FrameBufferType_BloomBlurH, FrameBufferDiction::FrameBufferType_BloomBlurV, RenderPassDiction::PipelineRenderPassBloomBlurV, { true, 1.2f, 1.0f });
	m_pBloomBlurHMaterial = GaussianBlurMaterial::CreateDefaultMaterial(FrameBufferDiction::FrameBufferType_BloomBlurV, FrameBufferDiction::FrameBufferType_BloomBlurH, RenderPassDiction::PipelineRenderPassBloomBlurH, { false, 1.2f, 1.0f });
	m_pPostProcessMaterial = PostProcessingMaterial::CreateDefaultMaterial();

	SimpleMaterialCreateInfo info = {};
	info.shaderPaths = { L"../data/shaders/sky_box.vert.spv", L"", L"", L"", L"../data/shaders/sky_box.frag.spv", L"" };
	info.materialUniformVars = {};
	info.vertexFormat = VertexFormatP;
	info.subpassIndex = 1;
	info.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading);
	info.depthWriteEnable = false;
	info.frameBufferType = FrameBufferDiction::FrameBufferType_Shading;

	m_pSkyBoxMaterial = ForwardMaterial::CreateDefaultMaterial(info);

	return true;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquirePBRMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = m_PBRGbufferMaterial->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << Scene);
	return pMaterialInstance;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquireShadowMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = m_pShadowMapMaterial->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << ShadowMapGen);
	return pMaterialInstance;
}

std::shared_ptr<MaterialInstance> RenderWorkManager::AcquireSkyBoxMaterialInstance() const
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = m_pSkyBoxMaterial->CreateMaterialInstance();
	pMaterialInstance->SetRenderMask(1 << Scene);
	return pMaterialInstance;
}

void RenderWorkManager::Draw(const std::shared_ptr<CommandBuffer>& pDrawCmdBuffer)
{
	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer));

	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y });

	m_PBRGbufferMaterial->OnPassStart();
	m_PBRGbufferMaterial->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_GBuffer));
	m_PBRGbufferMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->EndRenderPass(pDrawCmdBuffer);



	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShadowMap)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_ShadowMap));

	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetShadowGenWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetShadowGenWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetShadowGenWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetShadowGenWindowSize().y });

	m_pShadowMapMaterial->OnPassStart();
	m_pShadowMapMaterial->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_ShadowMap));
	m_pShadowMapMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShadowMap)->EndRenderPass(pDrawCmdBuffer);



	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAO)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAO));

	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().y });

	m_pSSAOMaterial->OnPassStart();
	m_pSSAOMaterial->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAO));
	m_pSSAOMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAO)->EndRenderPass(pDrawCmdBuffer);



	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurV)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurV));

	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().y });

	m_pSSAOBlurVMaterial->OnPassStart();
	m_pSSAOBlurVMaterial->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurV));
	m_pSSAOBlurVMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurV)->EndRenderPass(pDrawCmdBuffer);



	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurH)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurH));

	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize().y });

	m_pSSAOBlurHMaterial->OnPassStart();
	m_pSSAOBlurHMaterial->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_SSAOBlurH));
	m_pSSAOBlurHMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurH)->EndRenderPass(pDrawCmdBuffer);



	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Shading));

	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y });

	m_pShadingMaterial->OnPassStart();
	m_pShadingMaterial->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Shading));
	m_pShadingMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->NextSubpass(pDrawCmdBuffer);

	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y });

	m_pSkyBoxMaterial->OnPassStart();
	m_pSkyBoxMaterial->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_Shading));
	m_pSkyBoxMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->EndRenderPass(pDrawCmdBuffer);



	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_BloomBlurH));

	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().y });

	m_pBloomMaterial->OnPassStart();
	m_pBloomMaterial->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_BloomBlurH));
	m_pBloomMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->EndRenderPass(pDrawCmdBuffer);


	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloomBlurV)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_BloomBlurV));

	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().y });

	m_pBloomBlurVMaterial->OnPassStart();
	m_pBloomBlurVMaterial->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_BloomBlurV));
	m_pBloomBlurVMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloomBlurV)->EndRenderPass(pDrawCmdBuffer);


	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloomBlurH)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_BloomBlurH));

	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize().y });

	m_pBloomBlurHMaterial->OnPassStart();
	m_pBloomBlurHMaterial->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_BloomBlurH));
	m_pBloomBlurHMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloomBlurH)->EndRenderPass(pDrawCmdBuffer);



	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassPostProcessing)->BeginRenderPass(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_PostProcessing));

	GetGlobalVulkanStates()->SetViewport({ 0, 0, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y, 0, 1 });
	GetGlobalVulkanStates()->SetScissorRect({ 0, 0, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x, (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y });

	m_pPostProcessMaterial->OnPassStart();
	m_pPostProcessMaterial->Draw(pDrawCmdBuffer, FrameBufferDiction::GetInstance()->GetFrameBuffer(FrameBufferDiction::FrameBufferType_PostProcessing));
	m_pPostProcessMaterial->OnPassEnd();

	RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassPostProcessing)->EndRenderPass(pDrawCmdBuffer);
}