#pragma once

#include "../common/Singleton.h"
#include "../vulkan/RenderPass.h"
#include <map>

class RenderPassBase;
class ForwardRenderPass;
class DeferredShadingPass;

class RenderPassDiction : public Singleton<RenderPassDiction>
{
public:
	static const VkFormat OFFSCREEN_COLOR_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
	static const VkFormat OFFSCREEN_HDR_COLOR_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
	static const VkFormat OFFSCREEN_DEPTH_STENCIL_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT;
	static const VkFormat GBUFFER0_COLOR_FORMAT = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
	static const uint32_t OFFSCREEN_SIZE = 512;

	enum PipelineRenderPass
	{
		PipelineRenderPassGBuffer,
		PipelineRenderPassShadowMap,
		PipelineRenderPassSSAO,
		PipelineRenderPassShading,
		PipelineRenderPassPostProcessing,
		PipelineRenderPassCount
	};

public:
	bool Init() override;

public:

	std::shared_ptr<ForwardRenderPass> GetForwardRenderPass() const { return m_pForwardRenderPass; }
	std::shared_ptr<ForwardRenderPass> GetForwardRenderPassOffScreen() const { return m_pForwardRenderPassOffScreen; }

	std::shared_ptr<RenderPassBase> GetPipelineRenderPass(PipelineRenderPass pipelineRenderPass) { return m_pipelineRenderPasses[pipelineRenderPass]; }

protected:
	std::shared_ptr<ForwardRenderPass>		m_pForwardRenderPass;
	std::shared_ptr<ForwardRenderPass>		m_pForwardRenderPassOffScreen;

	std::vector<std::shared_ptr<RenderPassBase>>	m_pipelineRenderPasses;
};