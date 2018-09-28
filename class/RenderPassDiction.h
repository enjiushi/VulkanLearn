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
	enum PipelineRenderPass
	{
		PipelineRenderPassGBuffer,
		PipelineRenderPassMotionTileMax,
		PipelineRenderPassMotionNeighborMax,
		PipelineRenderPassShadowMap,
		PipelineRenderPassSSAO,
		PipelineRenderPassSSAOBlurV,
		PipelineRenderPassSSAOBlurH,
		PipelineRenderPassShading,
		PipelineRenderPassBloom,
		PipelineRenderPassBloomBlurV,
		PipelineRenderPassBloomBlurH,
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