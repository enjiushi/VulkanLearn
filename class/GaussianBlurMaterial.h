#pragma once
#include "Material.h"
#include "FrameBufferDiction.h"

class GaussianBlurMaterial : public Material
{
public:
	typedef struct _GaussianBlurParams
	{
		bool	isVertical = true;
		float	scale = 1.0f;
		float	strength = 1.0f;
	}GaussianBlurParams;

public:
	static std::shared_ptr<GaussianBlurMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo, const std::vector<std::shared_ptr<Image>>& inputTextures, GaussianBlurParams params);
	static std::shared_ptr<GaussianBlurMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo, FrameBufferDiction::FrameBufferType frameBufferType, GaussianBlurParams params);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer) override;

protected:
	bool Init(const std::shared_ptr<GaussianBlurMaterial>& pSelf,
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPassBase>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		const std::vector<VkPushConstantRange>& pushConstsRanges,
		const std::vector<UniformVar>& materialUniformVars,
		uint32_t vertexFormat,
		const std::vector<std::shared_ptr<Image>>& inputTextures,
		GaussianBlurParams params);

	void CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout) override;
	void CustomizePoolSize(std::vector<uint32_t>& counts) override;

protected:
	GaussianBlurParams	m_params;
};