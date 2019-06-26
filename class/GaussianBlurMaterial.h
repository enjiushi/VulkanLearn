#pragma once
#include "Material.h"
#include "FrameBufferDiction.h"
#include "RenderPassDiction.h"

class GaussianBlurMaterial : public Material
{
public:
	typedef struct _GaussianBlurParams
	{
		int		direction = 0;
		float	scale = 1.0f;
		float	strength = 1.0f;
	}GaussianBlurParams;

public:
	static std::shared_ptr<GaussianBlurMaterial> CreateDefaultMaterial(
		FrameBufferDiction::FrameBufferType inputFrameBufferType,
		FrameBufferDiction::FrameBufferType outputFrameBufferType,
		RenderPassDiction::PipelineRenderPass renderPass,
		GaussianBlurParams params);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0) override;
	void AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, uint32_t pingpong = 0) override;

protected:
	bool Init(const std::shared_ptr<GaussianBlurMaterial>& pSelf,
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPassBase>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		const std::vector<VkPushConstantRange>& pushConstsRanges,
		const std::vector<UniformVar>& materialUniformVars,
		uint32_t vertexFormat,
		uint32_t vertexFormatInMem,
		const std::vector<std::shared_ptr<Image>>& inputTextures,
		GaussianBlurParams params);

	void CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout) override;
	void CustomizePoolSize(std::vector<uint32_t>& counts) override;

protected:
	GaussianBlurParams					m_params;
	FrameBufferDiction::FrameBufferType m_inputFrameBufferType;
};