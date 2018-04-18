#pragma once

#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"
#include <map>

class FrameBuffer;

class FrameBufferDiction : public Singleton<FrameBufferDiction>
{
public:
	static const VkFormat OFFSCREEN_COLOR_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
	static const VkFormat OFFSCREEN_SINGLE_COLOR_FORMAT = VK_FORMAT_R8_UNORM;
	static const VkFormat OFFSCREEN_HDR_COLOR_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	static const VkFormat OFFSCREEN_DEPTH_STENCIL_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT;
	static const VkFormat OFFSCREEN_DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;
	static const VkFormat GBUFFER0_COLOR_FORMAT = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
	static const VkFormat SSAO_FORMAT = VK_FORMAT_R16_SFLOAT;
	static const VkFormat BLUR_FORMAT = VK_FORMAT_R16_SFLOAT;
	static const uint32_t OFFSCREEN_SIZE = 512;

	typedef std::vector<std::shared_ptr<FrameBuffer>> FrameBufferCombo;

public:
	enum FrameBufferType
	{
		FrameBufferType_GBuffer,
		FrameBufferType_ShadowMap,
		FrameBufferType_SSAO,
		FrameBufferType_GaussianBlur,
		FrameBufferType_Shading,
		FrameBufferType_PostProcessing,
		FrameBufferType_ForwardOffScreen,
		FrameBufferType_ForwardScreen,
		PipelineRenderPassCount
	};

	enum GBuffer
	{
		GBuffer0,
		GBuffer1,
		GBuffer2,
		GBufferCount
	};

public:
	bool Init() override;

public:
	FrameBufferCombo GetFrameBuffers(FrameBufferType type) { return m_frameBuffers[type]; }
	std::shared_ptr<FrameBuffer> GetFrameBuffer(FrameBufferType type);

	static VkFormat GetGBufferFormat(GBuffer gbuffer) { return m_GBufferFormatTable[gbuffer]; }

	FrameBufferCombo CreateGBufferFrameBuffer();
	FrameBufferCombo CreateShadowMapFrameBuffer();
	FrameBufferCombo CreateSSAOFrameBuffer();
	FrameBufferCombo CreateGaussianBlurFrameBuffer();
	FrameBufferCombo CreateShadingFrameBuffer();
	FrameBufferCombo CreatePostProcessingFrameBuffer();
	FrameBufferCombo CreateForwardOffScreenFrameBuffer();
	FrameBufferCombo CreateForwardScreenFrameBuffer();

protected:
	std::vector<FrameBufferCombo>	m_frameBuffers;
	static VkFormat					m_GBufferFormatTable[GBufferCount];
};