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

	m_pipelineRenderPasses.resize(PipelineRenderPassCount);

	for (uint32_t i = 0; i < PipelineRenderPassCount; i++)
	{
		switch ((PipelineRenderPass)i)
		{
		case  PipelineRenderPassGBuffer:
			m_pipelineRenderPasses[PipelineRenderPassGBuffer] = GBufferPass::Create(); break;
		case  PipelineRenderPassShadowMap:
			m_pipelineRenderPasses[PipelineRenderPassShadowMap] = ShadowMapPass::Create(); break;
		case PipelineRenderPassSSAO:
			m_pipelineRenderPasses[PipelineRenderPassSSAO] = SSAOPass::Create(FrameBufferDiction::SSAO_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); break;
		case PipelineRenderPassSSAOBlurV:
			m_pipelineRenderPasses[PipelineRenderPassSSAOBlurV] = GaussianBlurPass::Create(FrameBufferDiction::SSAO_FORMAT); break;
		case PipelineRenderPassSSAOBlurH:
			m_pipelineRenderPasses[PipelineRenderPassSSAOBlurH] = GaussianBlurPass::Create(FrameBufferDiction::SSAO_FORMAT); break;
		case PipelineRenderPassShading:
			m_pipelineRenderPasses[PipelineRenderPassShading] = DeferredShadingPass::Create(FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); break;
		case PipelineRenderPassBloom:
			m_pipelineRenderPasses[PipelineRenderPassBloom] = BloomPass::Create(); break;
		case PipelineRenderPassBloomBlurV:
			m_pipelineRenderPasses[PipelineRenderPassBloomBlurV] = GaussianBlurPass::Create(FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT); break;
		case PipelineRenderPassBloomBlurH:
			m_pipelineRenderPasses[PipelineRenderPassBloomBlurH] = GaussianBlurPass::Create(FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT); break;
		case PipelineRenderPassPostProcessing:
			m_pipelineRenderPasses[PipelineRenderPassPostProcessing] = PostProcessingPass::Create(FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); break;
		default:
			break;
		}
	}

	return true;
}