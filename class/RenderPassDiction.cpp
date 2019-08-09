#include "RenderPassDiction.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Framebuffer.h"
#include "../Maths/Vector.h"
#include "ForwardRenderPass.h"
#include "GBufferPass.h"
#include "DeferredShadingPass.h"
#include "FrameBufferDiction.h"
#include "CustomizedRenderPass.h"

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
		case  PipelineRenderPassMotionTileMax:
			m_pipelineRenderPasses[PipelineRenderPassMotionTileMax] = CustomizedRenderPass::Create({ { FrameBufferDiction::OFFSCREEN_MOTION_TILE_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,{ 0 } } }); break;
		case  PipelineRenderPassMotionNeighborMax:
			m_pipelineRenderPasses[PipelineRenderPassMotionNeighborMax] = CustomizedRenderPass::Create({ { FrameBufferDiction::OFFSCREEN_MOTION_TILE_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,{ 0 } } }); break;
		case  PipelineRenderPassShadowMap:
			m_pipelineRenderPasses[PipelineRenderPassShadowMap] = CustomizedRenderPass::Create({ { FrameBufferDiction::OFFSCREEN_DEPTH_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,{ 0 } } }); break;
		case PipelineRenderPassSSAOSSR:
			m_pipelineRenderPasses[PipelineRenderPassSSAOSSR] = CustomizedRenderPass::Create({ 
				{ FrameBufferDiction::SSAO_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,{ 0 } },
				{ FrameBufferDiction::SSR_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,{ 0, 0, 0, 0 } },
				}); break;
		case PipelineRenderPassSSAOBlurV:
			m_pipelineRenderPasses[PipelineRenderPassSSAOBlurV] = CustomizedRenderPass::Create({ { FrameBufferDiction::SSAO_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,{ 0, 0, 0, 0 } } }); break;
		case PipelineRenderPassSSAOBlurH:
			m_pipelineRenderPasses[PipelineRenderPassSSAOBlurH] = CustomizedRenderPass::Create({ { FrameBufferDiction::SSAO_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,{ 0, 0, 0, 0 } } }); break;
		case PipelineRenderPassShading:
			m_pipelineRenderPasses[PipelineRenderPassShading] = DeferredShadingPass::Create(FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); break;
		case PipelineRenderPassTemporalResolve:
			m_pipelineRenderPasses[PipelineRenderPassTemporalResolve] = CustomizedRenderPass::Create( { 
				{ FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, { 0, 0, 0, 0 } },
				{ FrameBufferDiction::COC_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, { 0 } } 
				}); break;
		case PipelineRenderPassDOF:
			m_pipelineRenderPasses[PipelineRenderPassDOF] = CustomizedRenderPass::Create({ { FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,{ 0, 0, 0, 0 } } }); break;
		case PipelineRenderPassBloom:
			m_pipelineRenderPasses[PipelineRenderPassBloom] = CustomizedRenderPass::Create({ { FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, { 0, 0, 0, 0} } }); break;
		case PipelineRenderPassCombine:
			m_pipelineRenderPasses[PipelineRenderPassCombine] = CustomizedRenderPass::Create({ { FrameBufferDiction::OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,{ 0, 0, 0, 0 } } }); break;
		case PipelineRenderPassPostProcessing:
			m_pipelineRenderPasses[PipelineRenderPassPostProcessing] = CustomizedRenderPass::Create({ { GetDevice()->GetPhysicalDevice()->GetSurfaceFormat().format, VK_IMAGE_LAYOUT_UNDEFINED , VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,{ 0, 0, 0, 0 } }}); break;
		default:
			break;
		}
	}

	return true;
}