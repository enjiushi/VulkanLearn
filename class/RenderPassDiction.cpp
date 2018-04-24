#include "RenderPassDiction.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Framebuffer.h"
#include "../Maths/Vector.h"
#include "ForwardRenderPass.h"
#include "GBufferPass.h"
#include "SSAOPass.h"
#include "DeferredShadingPass.h"
#include "PostProcessingPass.h"
#include "ShadowMapPass.h"
#include "GaussianBlurPass.h"
#include "BloomPass.h"
#include "FrameBufferDiction.h"

bool RenderPassDiction::Init()
{
	if (!Singleton<RenderPassDiction>::Init())
		return false;

	m_pForwardRenderPass = ForwardRenderPass::CreateForwardScreen();
	m_pForwardRenderPassOffScreen = ForwardRenderPass::CreateForwardOffScreen(FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	for (uint32_t i = 0; i < PipelineRenderPassCount; i++)
	{
		switch ((PipelineRenderPass)i)
		{
		case  PipelineRenderPassGBuffer:
			m_pipelineRenderPasses.push_back(GBufferPass::Create()); break;
		case  PipelineRenderPassShadowMap:
			m_pipelineRenderPasses.push_back(ShadowMapPass::Create()); break;
		case PipelineRenderPassSSAO:
			m_pipelineRenderPasses.push_back(SSAOPass::Create(FrameBufferDiction::SSAO_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)); break;
		case PipelineRenderPassGaussianBlurV:
			m_pipelineRenderPasses.push_back(GaussianBlurPass::Create(FrameBufferDiction::SSAO_FORMAT)); break;
		case PipelineRenderPassGaussianBlurH:
			m_pipelineRenderPasses.push_back(GaussianBlurPass::Create(FrameBufferDiction::SSAO_FORMAT)); break;
		case PipelineRenderPassShading:
			m_pipelineRenderPasses.push_back(DeferredShadingPass::Create(FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)); break;
		case PipelineRenderPassBloom:
			m_pipelineRenderPasses.push_back(BloomPass::Create()); break;
		case PipelineRenderPassPostProcessing:
			m_pipelineRenderPasses.push_back(PostProcessingPass::Create()); break;
		default:
			break;
		}
	}

	return true;
}