#pragma once
#include "Material.h"

class RenderPassBase;

class PostProcessingMaterial : public Material
{
protected:
	bool Init(const std::shared_ptr<PostProcessingMaterial>& pSelf,
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPassBase>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		const std::vector<UniformVar>& materialUniformVars,
		uint32_t vertexFormat);

	void CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout) override;
	void CustomizePoolSize(std::vector<uint32_t>& counts) override;

public:
	static std::shared_ptr<PostProcessingMaterial> CreateDefaultMaterial(uint32_t pingpong);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer) override;
	void AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer) override;

protected:
	uint32_t	m_pingPong;
};