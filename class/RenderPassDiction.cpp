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

bool RenderPassDiction::Init()
{
	if (!Singleton<RenderPassDiction>::Init())
		return false;

	m_pForwardRenderPass = ForwardRenderPass::CreateForwardScreen();
	m_pForwardRenderPassOffScreen = ForwardRenderPass::CreateForwardOffScreen({ OFFSCREEN_SIZE, OFFSCREEN_SIZE }, RenderPassBase::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	for (uint32_t i = 0; i < PipelineRenderPassCount; i++)
	{
		switch ((PipelineRenderPass)i)
		{
		case  PipelineRenderPassGBuffer:
			m_pipelineRenderPasses.push_back(GBufferPass::Create());
		case PipelineRenderPassSSAO:
			m_pipelineRenderPasses.push_back(SSAOPass::Create());
		case PipelineRenderPassShading:
			m_pipelineRenderPasses.push_back(DeferredShadingPass::Create(RenderPassBase::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL));
		case PipelineRenderPassPostProcessing:
			m_pipelineRenderPasses.push_back(PostProcessingPass::Create());
		default:
			break;
		}
	}

	return true;
}