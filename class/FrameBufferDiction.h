#pragma once

#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"
#include <map>

class FrameBuffer;
class Texture2D;

class FrameBufferDiction : public Singleton<FrameBufferDiction>
{
public:
	static const VkFormat OFFSCREEN_COLOR_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
	static const VkFormat OFFSCREEN_SINGLE_COLOR_FORMAT = VK_FORMAT_R8_UNORM;
	static const VkFormat OFFSCREEN_HDR_COLOR_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	static const VkFormat OFFSCREEN_DEPTH_STENCIL_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT;
	static const VkFormat OFFSCREEN_DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;
	static const VkFormat GBUFFER0_COLOR_FORMAT = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
	static const VkFormat OFFSCREEN_MOTION_TILE_FORMAT = VK_FORMAT_R16G16_SFLOAT;
	static const VkFormat SSAO_FORMAT = VK_FORMAT_R16_SFLOAT;
	static const VkFormat SSR_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	static const VkFormat BLUR_FORMAT = VK_FORMAT_R16_SFLOAT;
	static const VkFormat COC_FORMAT = VK_FORMAT_R16_SFLOAT;

	static const uint32_t WINDOW_WIDTH = 1440;
	static const uint32_t WINDOW_HEIGHT = 1024;
	static const uint32_t ENV_GEN_WINDOW_SIZE = 512;
	static const uint32_t SHADOW_GEN_WINDOW_SIZE = 1024;
	static const uint32_t SSAO_SSR_WINDOW_WIDTH = WINDOW_WIDTH / 2;
	static const uint32_t SSAO_SSR_WINDOW_HEIGHT = WINDOW_HEIGHT / 2;
	static const uint32_t BLOOM_WINDOW_SIZE = 256;
	static const uint32_t MOTION_TILE_SIZE = 16;

	typedef std::vector<std::shared_ptr<FrameBuffer>> FrameBufferCombo;

public:
	enum FrameBufferType
	{
		FrameBufferType_GBuffer,
		FrameBufferType_MotionTileMax,
		FrameBufferType_MotionNeighborMax,
		FrameBufferType_ShadowMap,
		FrameBufferType_SSAOSSR,
		FrameBufferType_SSAOBlurV,
		FrameBufferType_SSAOBlurH,
		FrameBufferType_Shading,
		FrameBufferType_TemporalResolve,
		FrameBufferType_Bloom,
		FrameBufferType_CombineResult,
		FrameBufferType_PostProcessing,
		FrameBufferType_EnvGenOffScreen,
		FrameBufferType_ForwardScreen,
		PipelineRenderPassCount
	};

	enum GBuffer
	{
		GBuffer0,
		GBuffer1,
		GBuffer2,
		GBuffer3,
		MotionVector,
		GBufferCount
	};

	enum TemporalFrameBuffer
	{
		ShadingResult,
		CoC,
		TemporalFrameBufferCount
	};

public:
	bool Init() override;

public:
	FrameBufferCombo GetFrameBuffers(FrameBufferType type, uint32_t layer = 0);
	std::shared_ptr<FrameBuffer> GetFrameBuffer(FrameBufferType type, uint32_t layer = 0);
	std::shared_ptr<FrameBuffer> GetPingPongFrameBuffer(FrameBufferType type, uint32_t pingPongIndex);
	std::shared_ptr<FrameBuffer> GetPingPongFrameBuffer(FrameBufferType type, uint32_t frameIndex, uint32_t pingPongIndex);

	static VkFormat GetGBufferFormat(GBuffer gbuffer) { return m_GBufferFormatTable[gbuffer]; }

	FrameBufferCombo CreateGBufferFrameBuffer();
	FrameBufferCombo CreateMotionTileMaxFrameBuffer();
	FrameBufferCombo CreateMotionNeighborMaxFrameBuffer();
	FrameBufferCombo CreateShadowMapFrameBuffer();
	FrameBufferCombo CreateSSAOSSRFrameBuffer();
	FrameBufferCombo CreateSSAOBlurFrameBufferV();
	FrameBufferCombo CreateSSAOBlurFrameBufferH();
	FrameBufferCombo CreateShadingFrameBuffer();
	FrameBufferCombo CreateTemporalResolveFrameBuffer();
	FrameBufferCombo CreateBloomFrameBuffer(uint32_t layer = 0);
	FrameBufferCombo CreateCombineResultFrameBuffer();
	FrameBufferCombo CreatePostProcessingFrameBuffer();
	FrameBufferCombo CreateForwardEnvGenOffScreenFrameBuffer();
	FrameBufferCombo CreateForwardScreenFrameBuffer();

protected:
	// Since you can't render to a specific mip layer of a frame buffer
	// I'll have to create multiple frame buffers as layers
	// But, most of frame buffers contain 1 layer
	std::vector<std::vector<FrameBufferCombo>>					m_frameBuffers;
	std::vector<std::vector<std::shared_ptr<Texture2D>>>		m_temporalTexture;
	static VkFormat												m_GBufferFormatTable[GBufferCount];
};