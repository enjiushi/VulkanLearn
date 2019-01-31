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
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R16G16_SFLOAT,
};

bool FrameBufferDiction::Init()
{
	if (!Singleton<FrameBufferDiction>::Init())
		return false;

	m_frameBuffers.resize(PipelineRenderPassCount);

	for (uint32_t i = 0; i < PipelineRenderPassCount; i++)
	{
		m_frameBuffers[i].resize(1);

		switch ((FrameBufferType)i)
		{
		case  FrameBufferType_GBuffer:
			m_frameBuffers[FrameBufferType_GBuffer][0] = CreateGBufferFrameBuffer(); break;
		case  FrameBufferType_MotionTileMax:
			m_frameBuffers[FrameBufferType_MotionTileMax][0] = CreateMotionTileMaxFrameBuffer(); break;
		case  FrameBufferType_MotionNeighborMax:
			m_frameBuffers[FrameBufferType_MotionNeighborMax][0] = CreateMotionNeighborMaxFrameBuffer(); break;
		case  FrameBufferType_ShadowMap:
			m_frameBuffers[FrameBufferType_ShadowMap][0] = CreateShadowMapFrameBuffer(); break;
		case FrameBufferType_SSAOSSR:
			m_frameBuffers[FrameBufferType_SSAOSSR][0] = CreateSSAOSSRFrameBuffer(); break;
		case FrameBufferType_SSAOBlurV:
			m_frameBuffers[FrameBufferType_SSAOBlurV][0] = CreateSSAOBlurFrameBufferV(); break;
		case FrameBufferType_SSAOBlurH:
			m_frameBuffers[FrameBufferType_SSAOBlurH][0] = CreateSSAOBlurFrameBufferH(); break;
		case FrameBufferType_Shading:
			m_frameBuffers[FrameBufferType_Shading][0] = CreateShadingFrameBuffer(); break;
		case FrameBufferType_TemporalResolve:
			m_frameBuffers[FrameBufferType_TemporalResolve][0] = CreateTemporalResolveFrameBuffer(); break;
		case FrameBufferType_Bloom:
			m_frameBuffers[FrameBufferType_Bloom][0] = CreateBloomFrameBuffer(); break;
		case FrameBufferType_CombineResult:
			m_frameBuffers[FrameBufferType_CombineResult][0] = CreateCombineResultFrameBuffer(); break;
		case FrameBufferType_PostProcessing:
			m_frameBuffers[FrameBufferType_PostProcessing][0] = CreatePostProcessingFrameBuffer(); break;
		case FrameBufferType_EnvGenOffScreen:
			m_frameBuffers[FrameBufferType_EnvGenOffScreen][0] = CreateForwardEnvGenOffScreenFrameBuffer(); break;
		case FrameBufferType_ForwardScreen:
			m_frameBuffers[FrameBufferType_ForwardScreen][0] = CreateForwardScreenFrameBuffer(); break;
		default:
			break;
		}
	}

	return true;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::GetFrameBuffers(FrameBufferType type, uint32_t layer)
{ 
	// Only bloom is layered, might generalize it through all frame buffers, when needed
	if (m_frameBuffers[type].size() <= layer && type == FrameBufferType_Bloom)
	{
		for (uint32_t i = 0; i < layer - m_frameBuffers[type].size() + 1; i++)
			m_frameBuffers[type].push_back(CreateBloomFrameBuffer(m_frameBuffers[type].size()));
	}
	return m_frameBuffers[type][layer];
}

std::shared_ptr<FrameBuffer> FrameBufferDiction::GetFrameBuffer(FrameBufferType type, uint32_t layer)
{
	// Only bloom is layered, might generalize it through all frame buffers, when needed
	if (m_frameBuffers[type].size() <= layer && type == FrameBufferType_Bloom)
	{
		for (uint32_t i = 0; i < layer - m_frameBuffers[type].size() + 1; i++)
			m_frameBuffers[type].push_back(CreateBloomFrameBuffer(m_frameBuffers[type].size()));
	}
	return m_frameBuffers[type][layer][FrameMgr()->FrameIndex()];
}

std::shared_ptr<FrameBuffer> FrameBufferDiction::GetPingPongFrameBuffer(FrameBufferType type, uint32_t pingPongIndex)
{
	return GetPingPongFrameBuffer(type, FrameMgr()->FrameIndex(), pingPongIndex);
}

std::shared_ptr<FrameBuffer> FrameBufferDiction::GetPingPongFrameBuffer(FrameBufferType type, uint32_t frameIndex, uint32_t pingPongIndex)
{
	if (frameIndex >= GetSwapChain()->GetSwapChainImageCount())
		return nullptr;

	if (type != FrameBufferType_TemporalResolve)
		return m_frameBuffers[type][0][frameIndex];

	if (frameIndex % 2 != pingPongIndex)
		return m_frameBuffers[type][0][frameIndex + 3];
	else
		return m_frameBuffers[type][0][frameIndex];
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
		gbuffer_vec[GBuffer3] = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, m_GBufferFormatTable[GBuffer3]);
		gbuffer_vec[MotionVector] = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, m_GBufferFormatTable[MotionVector]);

		std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer = DepthStencilBuffer::CreateSampledAttachment(GetDevice(), OFFSCREEN_DEPTH_STENCIL_FORMAT, windowSize.x, windowSize.y);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), gbuffer_vec, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateMotionTileMaxFrameBuffer()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetMotionTileWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), (uint32_t)windowSize.x, (uint32_t)windowSize.y, OFFSCREEN_MOTION_TILE_FORMAT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionTileMax)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateMotionNeighborMaxFrameBuffer()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetMotionTileWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), (uint32_t)windowSize.x, (uint32_t)windowSize.y, OFFSCREEN_MOTION_TILE_FORMAT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionNeighborMax)->GetRenderPass()));
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

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateSSAOSSRFrameBuffer()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pSSAO = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, SSAO_FORMAT);
		std::shared_ptr<Image> pSSR = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, SSR_FORMAT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pSSAO, pSSR }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOSSR)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateSSAOBlurFrameBufferV()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize();

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
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize();

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
		std::shared_ptr<DepthStencilBuffer> pDepthStencilBuffer = m_frameBuffers[FrameBufferType_GBuffer][0][i]->GetDepthStencilTarget();
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->GetRenderPass()));
	}

	return frameBuffers;
}


FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateTemporalResolveFrameBuffer()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();

	std::vector<std::shared_ptr<Image>> temporalShadingResult =
	{
		Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	};

	std::vector<std::shared_ptr<Image>> temporalCoC =
	{
		Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, COC_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, COC_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	};

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount() * 2; i++)
	{
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(),
			{
				temporalShadingResult[i % 2],
				temporalCoC[i % 2],
			}, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassTemporalResolve)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateBloomFrameBuffer(uint32_t layer)
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();

#if defined(_DEBUG)
	uint32_t min = std::fminf(windowSize.x, windowSize.y);
	uint32_t layerMin = log2(min);
	ASSERTION(layerMin != 1);
#endif

	float div = std::powf(2.0f, layer);
	Vector2f layerSize = { windowSize.x / div, windowSize.y / div };

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), layerSize.x, layerSize.y, OFFSCREEN_HDR_COLOR_FORMAT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateCombineResultFrameBuffer()
{
	Vector2f windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Texture2D::CreateOffscreenTexture(GetDevice(), windowSize.x, windowSize.y, OFFSCREEN_HDR_COLOR_FORMAT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassCombine)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreatePostProcessingFrameBuffer()
{
	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pImg = GetSwapChain()->GetSwapChainImage(i);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pImg }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassPostProcessing)->GetRenderPass()));
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