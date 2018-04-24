#include "FrameBufferDiction.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/Texture2D.h"
#include "../vulkan/DepthStencilBuffer.h"
#include "../vulkan/Framebuffer.h"
#include "../Maths/Vector.h"
#include "RenderPassDiction.h"
#include "RenderPassBase.h"
#include "ForwardRenderPass.h"
#include "UniformData.h"

VkFormat FrameBufferDiction::m_GBufferFormatTable[FrameBufferDiction::GBufferCount] =
{
	VK_FORMAT_A2R10G10B10_UNORM_PACK32,
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_R8G8B8A8_UNORM
};

bool FrameBufferDiction::Init()
{
	if (!Singleton<FrameBufferDiction>::Init())
		return false;

	for (uint32_t i = 0; i < PipelineRenderPassCount; i++)
	{
		switch ((FrameBufferType)i)
		{
		case  FrameBufferType_GBuffer:
			m_frameBuffers.push_back(CreateGBufferFrameBuffer()); break;
		case  FrameBufferType_ShadowMap:
			m_frameBuffers.push_back(CreateShadowMapFrameBuffer()); break;
		case FrameBufferType_SSAO:
			m_frameBuffers.push_back(CreateSSAOFrameBuffer()); break;
		case FrameBufferType_SSAOBlurV:
			m_frameBuffers.push_back(CreateSSAOBlurFrameBufferV()); break;
		case FrameBufferType_SSAOBlurH:
			m_frameBuffers.push_back(CreateSSAOBlurFrameBufferH()); break;
		case FrameBufferType_Shading:
			m_frameBuffers.push_back(CreateShadingFrameBuffer()); break;
		case FrameBufferType_BloomV:
			m_frameBuffers.push_back(CreateBloomFrameBufferV()); break;
		case FrameBufferType_BloomH:
			m_frameBuffers.push_back(CreateBloomFrameBufferH()); break;
		case FrameBufferType_PostProcessing:
			m_frameBuffers.push_back(CreatePostProcessingFrameBuffer()); break;
		case FrameBufferType_EnvGenOffScreen:
			m_frameBuffers.push_back(CreateForwardEnvGenOffScreenFrameBuffer()); break;
		case FrameBufferType_ForwardScreen:
			m_frameBuffers.push_back(CreateForwardScreenFrameBuffer()); break;
		default:
			break;
		}
	}

	return true;
}

std::shared_ptr<FrameBuffer> FrameBufferDiction::GetFrameBuffer(FrameBufferType type)
{
	return m_frameBuffers[type][FrameMgr()->FrameIndex()];
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateGBufferFrameBuffer()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::vector<std::shared_ptr<Image>> gbuffer_vec(GBufferCount);

		gbuffer_vec[GBuffer0] = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, m_GBufferFormatTable[GBuffer0]);
		gbuffer_vec[GBuffer1] = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, m_GBufferFormatTable[GBuffer1]);
		gbuffer_vec[GBuffer2] = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, m_GBufferFormatTable[GBuffer2]);

		std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer = DepthStencilBuffer::CreateSampledAttachment(GetDevice(), OFFSCREEN_DEPTH_STENCIL_FORMAT, windowSize.x, windowSize.y);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), gbuffer_vec, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateShadowMapFrameBuffer()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetShadowGenWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer = DepthStencilBuffer::CreateSampledAttachment(GetDevice(), OFFSCREEN_DEPTH_FORMAT, windowSize.x, windowSize.y);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), std::vector<std::shared_ptr<Image>>(), pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShadowMap)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateSSAOFrameBuffer()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, SSAO_FORMAT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAO)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateSSAOBlurFrameBufferV()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, SSAO_FORMAT);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), pColorTarget, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurV)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateSSAOBlurFrameBufferH()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, SSAO_FORMAT);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), pColorTarget, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurH)->GetRenderPass()));
	}

	return frameBuffers;
}


FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateShadingFrameBuffer()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, OFFSCREEN_HDR_COLOR_FORMAT);
		std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer = m_frameBuffers[FrameBufferType_GBuffer][i]->GetDepthStencilTarget();
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget , GetSwapChain()->GetSwapChainImage(i) }, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateBloomFrameBufferV()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, OFFSCREEN_HDR_COLOR_FORMAT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloomBlurV)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateBloomFrameBufferH()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetBloomWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, OFFSCREEN_HDR_COLOR_FORMAT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloomBlurH)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreatePostProcessingFrameBuffer()
{
	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.width, GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.height, BLUR_FORMAT);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), pColorTarget, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAO)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateForwardEnvGenOffScreenFrameBuffer()
{
	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x, UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().y, OFFSCREEN_HDR_COLOR_FORMAT);

		std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer = DepthStencilBuffer::CreateSampledAttachment(GetDevice(), OFFSCREEN_DEPTH_STENCIL_FORMAT, GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.width, GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.height);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), pColorTarget, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetForwardRenderPassOffScreen()->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateForwardScreenFrameBuffer()
{
	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = GetSwapChain()->GetSwapChainImage(0);

		std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer = DepthStencilBuffer::CreateSampledAttachment(GetDevice(), OFFSCREEN_DEPTH_STENCIL_FORMAT, GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.width, GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.height);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), pColorTarget, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetForwardRenderPass()->GetRenderPass()));
	}

	return frameBuffers;
}