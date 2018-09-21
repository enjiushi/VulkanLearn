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
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_R16G16_SNORM
};

bool FrameBufferDiction::Init()
{
	if (!Singleton<FrameBufferDiction>::Init())
		return false;

	m_frameBuffers.resize(PipelineRenderPassCount);

	for (uint32_t i = 0; i < PipelineRenderPassCount; i++)
	{
		switch ((FrameBufferType)i)
		{
		case  FrameBufferType_GBuffer:
			m_frameBuffers[FrameBufferType_GBuffer] = CreateGBufferFrameBuffer(); break;
		case  FrameBufferType_ShadowMap:
			m_frameBuffers[FrameBufferType_ShadowMap] = CreateShadowMapFrameBuffer(); break;
		case FrameBufferType_SSAO:
			m_frameBuffers[FrameBufferType_SSAO] = CreateSSAOFrameBuffer(); break;
		case FrameBufferType_SSAOBlurV:
			m_frameBuffers[FrameBufferType_SSAOBlurV] = CreateSSAOBlurFrameBufferV(); break;
		case FrameBufferType_SSAOBlurH:
			m_frameBuffers[FrameBufferType_SSAOBlurH] = CreateSSAOBlurFrameBufferH(); break;
		case FrameBufferType_Shading:
			m_frameBuffers[FrameBufferType_Shading] = CreateShadingFrameBuffer(); break;
		case FrameBufferType_Temporal0:
			m_frameBuffers[FrameBufferType_Temporal0] = CreateShadingTemporalFrameBuffer(); break;
		case FrameBufferType_Temporal1:
			m_frameBuffers[FrameBufferType_Temporal1] = CreateShadingTemporalFrameBuffer(); break;
		case FrameBufferType_BloomBlurV:
			m_frameBuffers[FrameBufferType_BloomBlurV] = CreateBloomFrameBufferV(); break;
		case FrameBufferType_BloomBlurH:
			m_frameBuffers[FrameBufferType_BloomBlurH] = CreateBloomFrameBufferH(); break;
		case FrameBufferType_PostProcessing:
			m_frameBuffers[FrameBufferType_PostProcessing] = CreatePostProcessingFrameBuffer(); break;
		case FrameBufferType_EnvGenOffScreen:
			m_frameBuffers[FrameBufferType_EnvGenOffScreen] = CreateForwardEnvGenOffScreenFrameBuffer(); break;
		case FrameBufferType_ForwardScreen:
			m_frameBuffers[FrameBufferType_ForwardScreen] = CreateForwardScreenFrameBuffer(); break;
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

std::shared_ptr<FrameBuffer> FrameBufferDiction::GetFrameBuffer(FrameBufferType type, uint32_t pingPongIndex)
{
	return GetFrameBuffer(type, FrameMgr()->FrameIndex(), pingPongIndex);
}

std::shared_ptr<FrameBuffer> FrameBufferDiction::GetFrameBuffer(FrameBufferType type, uint32_t frameIndex, uint32_t pingPongIndex)
{
	if (frameIndex >= GetSwapChain()->GetSwapChainImageCount())
		return nullptr;

	if (type != FrameBufferType_PostProcessing)
		return m_frameBuffers[type][frameIndex];

	if (frameIndex % 2 != pingPongIndex)
		return m_frameBuffers[type][frameIndex + 3];
	else
		return m_frameBuffers[type][frameIndex];
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
		gbuffer_vec[MotionVector] = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, m_GBufferFormatTable[MotionVector]);

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
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateShadingTemporalFrameBuffer()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();

	FrameBufferCombo frameBuffers;

	std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassTemporal)->GetRenderPass()));

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
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();

	m_temporalTexture.push_back(Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
	m_temporalTexture.push_back(Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount() * 2; i++)
	{
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { GetSwapChain()->GetSwapChainImage(i % GetSwapChain()->GetSwapChainImageCount()), m_temporalTexture[i % 2] }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassPostProcessing)->GetRenderPass()));
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
		std::shared_ptr<Image> pColorTarget = GetSwapChain()->GetSwapChainImage(i);

		std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer = DepthStencilBuffer::CreateSampledAttachment(GetDevice(), OFFSCREEN_DEPTH_STENCIL_FORMAT, GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.width, GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.height);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), pColorTarget, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetForwardRenderPass()->GetRenderPass()));
	}

	return frameBuffers;
}