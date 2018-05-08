#include "RenderWorkManager.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/DepthStencilBuffer.h"
#include "../vulkan/Texture2D.h"
#include "RenderPassDiction.h"
#include "ForwardRenderPass.h"
#include "DeferredMaterial.h"
#include "ShadowMapMaterial.h"
#include "SSAOMaterial.h"
#include "GaussianBlurMaterial.h"
#include "BloomMaterial.h"
#include "ForwardMaterial.h"

bool RenderWorkManager::Init()
{
	if (!Singleton<RenderWorkManager>::Init())
		return false;
	
	//std::vector<UniformVar> pbr_material_vars =
	//{
	//	{
	//		{
	//			Vec4Unit,
	//			"Albedo Roughness"
	//		},
	//		{
	//			Vec2Unit,
	//			"AO Metalic"
	//		},
	//		{
	//			OneUnit,
	//			"Albedo Roughness Texture Index"
	//		},
	//		{
	//			OneUnit,
	//			"Normal AO Texture Index"
	//		},
	//		{
	//			OneUnit,
	//			"Metallic Texture Index"
	//		}
	//	}
	//};

	//SimpleMaterialCreateInfo info = {};
	//info.shaderPaths = { L"../data/shaders/pbr_gbuffer_gen.vert.spv", L"", L"", L"", L"../data/shaders/pbr_gbuffer_gen.frag.spv", L"" };
	//info.vertexBindingsInfo = { m_pGunMesh->GetVertexBuffer()->GetBindingDesc() };	// FIXME: I need to extract shared vertex buffer out of specific mesh
	//info.vertexAttributesInfo = m_pGunMesh->GetVertexBuffer()->GetAttribDesc();
	//info.materialUniformVars = pbr_material_vars;
	//info.vertexFormat = m_pGunMesh->GetVertexBuffer()->GetVertexFormat();
	//info.subpassIndex = 0;
	//info.frameBufferType = FrameBufferDiction::FrameBufferType_GBuffer;
	//info.pRenderPass = RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer);


	//m_PBRGbufferMaterial = GBufferMaterial::CreateDefaultMaterial(info);

	return true;
}

void RenderWorkManager::Draw()
{

}