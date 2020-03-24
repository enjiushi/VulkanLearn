#include "FrameBufferDiction.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/Image.h"
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
	VK_FORMAT_R16G16_SFLOAT,
};

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateFrameBuffer(FrameBufferType type, uint32_t layer)
{
	switch (type)
	{
	case  FrameBufferType_GBuffer:
		return CreateGBufferFrameBuffer(layer);
	case  FrameBufferType_MotionTileMax:
		return CreateMotionTileMaxFrameBuffer(layer);
	case  FrameBufferType_MotionNeighborMax:
		return CreateMotionNeighborMaxFrameBuffer(layer);
	case  FrameBufferType_ShadowMap:
		return CreateShadowMapFrameBuffer(layer);
	case FrameBufferType_SSAOSSR:
		return CreateSSAOSSRFrameBuffer(layer);
	case FrameBufferType_SSAOBlurV:
		return CreateSSAOBlurFrameBufferV(layer);
	case FrameBufferType_SSAOBlurH:
		return CreateSSAOBlurFrameBufferH(layer);
	case FrameBufferType_Shading:
		return CreateShadingFrameBuffer(layer);
	case FrameBufferType_TemporalResolve:
		return CreateTemporalResolveFrameBuffer(layer);
	case FrameBufferType_DOF:
		return CreateDOFFrameBuffer(layer);
	case FrameBufferType_Bloom:
		return CreateBloomFrameBuffer(layer);
	case FrameBufferType_CombineResult:
		return CreateCombineResultFrameBuffer(layer);
	case FrameBufferType_PostProcessing:
		return CreatePostProcessingFrameBuffer(layer);
	case FrameBufferType_EnvGenOffScreen:
		return CreateForwardEnvGenOffScreenFrameBuffer(layer);
	case FrameBufferType_ForwardScreen:
		return CreateForwardScreenFrameBuffer(layer);
	default:
		ASSERTION(false);
		break;
	}

	return {};
}

bool FrameBufferDiction::Init()
{
	if (!Singleton<FrameBufferDiction>::Init())
		return false;

	m_frameBuffers.resize(PipelineRenderPassCount);

	for (uint32_t i = 0; i < PipelineRenderPassCount; i++)
		m_frameBuffers[i].push_back(CreateFrameBuffer((FrameBufferType)i));

	return true;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::GetFrameBuffers(FrameBufferType type, uint32_t layer)
{ 
	// Only bloom is layered, might generalize it through all frame buffers, when needed
	if (m_frameBuffers[type].size() <= layer)
	{
		uint32_t remain = layer - (uint32_t)m_frameBuffers[type].size() + 1;
		for (uint32_t i = 0; i < remain; i++)
			m_frameBuffers[type].push_back(CreateFrameBuffer(type, (uint32_t)m_frameBuffers[type].size()));
	}
	return m_frameBuffers[type][layer];
}

std::shared_ptr<FrameBuffer> FrameBufferDiction::GetFrameBuffer(FrameBufferType type, uint32_t layer)
{
	// Only bloom is layered, might generalize it through all frame buffers, when needed
	if (m_frameBuffers[type].size() <= layer)
	{
		for (uint32_t i = 0; i < layer - m_frameBuffers[type].size() + 1; i++)
			m_frameBuffers[type].push_back(CreateFrameBuffer(type, (uint32_t)m_frameBuffers[type].size()));
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

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateGBufferFrameBuffer(uint32_t layer)
{
	Vector2ui size =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y,
	};

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::vector<std::shared_ptr<Image>> gbuffer_vec(GBufferCount);

		gbuffer_vec[GBuffer0] = Image::CreateOffscreenTexture2D(GetDevice(), size, m_GBufferFormatTable[GBuffer0]);
		gbuffer_vec[GBuffer1] = Image::CreateOffscreenTexture2D(GetDevice(), size, m_GBufferFormatTable[GBuffer1]);
		gbuffer_vec[GBuffer2] = Image::CreateOffscreenTexture2D(GetDevice(), size, m_GBufferFormatTable[GBuffer2]);
		gbuffer_vec[MotionVector] = Image::CreateOffscreenTexture2D(GetDevice(), size, m_GBufferFormatTable[MotionVector]);

		std::shared_ptr<Image> pDepthStencilBuffer = Image::CreateDepthStencilSampledAttachment(GetDevice(), OFFSCREEN_DEPTH_STENCIL_FORMAT, size);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), gbuffer_vec, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassGBuffer)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateMotionTileMaxFrameBuffer(uint32_t layer)
{
	Vector2ui size =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetMotionTileWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetMotionTileWindowSize().y,
	};

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_MOTION_TILE_FORMAT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionTileMax)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateMotionNeighborMaxFrameBuffer(uint32_t layer)
{
	Vector2ui size =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetMotionTileWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetMotionTileWindowSize().y,
	};

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_MOTION_TILE_FORMAT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassMotionNeighborMax)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateShadowMapFrameBuffer(uint32_t layer)
{
	Vector2d windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetShadowGenWindowSize();

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pDepthStencilBuffer = Image::CreateDepthStencilSampledAttachment(GetDevice(), OFFSCREEN_DEPTH_FORMAT, { (uint32_t)windowSize.x, (uint32_t)windowSize.y });

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), std::vector<std::shared_ptr<Image>>(), pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShadowMap)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateSSAOSSRFrameBuffer(uint32_t layer)
{
	Vector2ui size =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().y,
	};

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pSSAO = Image::CreateOffscreenTexture2D(GetDevice(), size, SSAO_FORMAT);
		std::shared_ptr<Image> pSSR = Image::CreateOffscreenTexture2D(GetDevice(), size, SSR_FORMAT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pSSAO, pSSR }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOSSR)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateSSAOBlurFrameBufferV(uint32_t layer)
{
	Vector2ui size =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().y,
	};

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Image::CreateOffscreenTexture2D(GetDevice(), size, SSAO_FORMAT);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), pColorTarget, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurV)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateSSAOBlurFrameBufferH(uint32_t layer)
{
	Vector2ui size =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetSSAOSSRWindowSize().y,
	};

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Image::CreateOffscreenTexture2D(GetDevice(), size, SSAO_FORMAT);

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), pColorTarget, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassSSAOBlurH)->GetRenderPass()));
	}

	return frameBuffers;
}


FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateShadingFrameBuffer(uint32_t layer)
{
	Vector2ui size =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y,
	};

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pShadingResult = Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT);
		std::shared_ptr<Image> pSSResult = Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT);
		std::shared_ptr<Image> pDepthStencilBuffer = m_frameBuffers[FrameBufferType_GBuffer][0][i]->GetDepthStencilTarget();
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pShadingResult, pSSResult }, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassShading)->GetRenderPass()));
	}

	return frameBuffers;
}


FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateTemporalResolveFrameBuffer(uint32_t layer)
{
	Vector2ui size =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y,
	};

	std::vector<std::shared_ptr<Image>> temporalShadingResult =
	{
		Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	};

	std::vector<std::shared_ptr<Image>> temporalSSRResult =
	{
		Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	};

	std::vector<std::shared_ptr<Image>> temporalResult =
	{
		Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	};

	std::vector<std::shared_ptr<Image>> temporalCoC =
	{
		Image::CreateOffscreenTexture2D(GetDevice(), size, COC_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
		Image::CreateOffscreenTexture2D(GetDevice(), size, COC_FORMAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	};

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount() * 2; i++)
	{
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(),
			{
				temporalShadingResult[i % 2],
				temporalSSRResult[i % 2],
				temporalResult[i % 2],
				temporalCoC[i % 2],
			}, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassTemporalResolve)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateDOFFrameBuffer(uint32_t layer)
{
	Vector2d windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();

	uint32_t div = (layer == CombineLayer ? 1 : 2);
	Vector2d layerSize = { windowSize.x / div, windowSize.y / div };

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget;
		if (layer == PostfilterLayer)
			pColorTarget = GetFrameBuffers(FrameBufferType_DOF, PrefilterLayer)[i]->GetColorTarget(0);
		else
		{
			Vector2ui size =
			{
				(uint32_t)layerSize.x,
				(uint32_t)layerSize.y,
			};
			pColorTarget = Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT);
		}
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassDOF)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateBloomFrameBuffer(uint32_t layer)
{
	Vector2d windowSize = UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize();

#if defined(_DEBUG)
	uint32_t min = (uint32_t)std::fmin(windowSize.x, windowSize.y);
	uint32_t layerMin = (uint32_t)log2(min);
	ASSERTION(layerMin != 1);
#endif

	double div = std::pow(2.0, layer);
	Vector2d layerSize = { windowSize.x / div, windowSize.y / div };

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		Vector2ui size =
		{
			(uint32_t)layerSize.x,
			(uint32_t)layerSize.y,
		};
		std::shared_ptr<Image> pColorTarget = Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_USAGE_STORAGE_BIT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassBloom)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateCombineResultFrameBuffer(uint32_t layer)
{
	Vector2ui size =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().y,
	};

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_USAGE_STORAGE_BIT);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pColorTarget }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassCombine)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreatePostProcessingFrameBuffer(uint32_t layer)
{
	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pImg = GetSwapChain()->GetSwapChainImage(i);
		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), { pImg }, nullptr, RenderPassDiction::GetInstance()->GetPipelineRenderPass(RenderPassDiction::PipelineRenderPassPostProcessing)->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateForwardEnvGenOffScreenFrameBuffer(uint32_t layer)
{
	Vector2ui size =
	{
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().x,
		(uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetEnvGenWindowSize().y,
	};

	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = Image::CreateOffscreenTexture2D(GetDevice(), size, OFFSCREEN_HDR_COLOR_FORMAT);

		std::shared_ptr<Image> pDepthStencilBuffer = Image::CreateDepthStencilSampledAttachment(GetDevice(), OFFSCREEN_DEPTH_STENCIL_FORMAT, { GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.width, GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.height });

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), pColorTarget, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetForwardRenderPassOffScreen()->GetRenderPass()));
	}

	return frameBuffers;
}

FrameBufferDiction::FrameBufferCombo FrameBufferDiction::CreateForwardScreenFrameBuffer(uint32_t layer)
{
	FrameBufferCombo frameBuffers;

	for (uint32_t i = 0; i < GetSwapChain()->GetSwapChainImageCount(); i++)
	{
		std::shared_ptr<Image> pColorTarget = GetSwapChain()->GetSwapChainImage(i);

		std::shared_ptr<Image> pDepthStencilBuffer = Image::CreateDepthStencilSampledAttachment(GetDevice(), OFFSCREEN_DEPTH_STENCIL_FORMAT, { GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.width, GetSwapChain()->GetSwapChainImage(0)->GetImageInfo().extent.height });

		frameBuffers.push_back(FrameBuffer::Create(GetDevice(), pColorTarget, pDepthStencilBuffer, RenderPassDiction::GetInstance()->GetForwardRenderPass()->GetRenderPass()));
	}

	return frameBuffers;
}