#pragma once
#include "Material.h"
#include "FrameBufferDiction.h"

class GaussianBlurMaterial : public Material
{
public:
	static std::shared_ptr<GaussianBlurMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer) override;

	void SetDirection(bool isVertical) { m_isVertical = isVertical; }
	bool GetDirection() const { return m_isVertical; }

protected:
	bool Init(const std::shared_ptr<GaussianBlurMaterial>& pSelf,
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPassBase>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		const std::vector<UniformVar>& materialUniformVars,
		uint32_t vertexFormat);

	void CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout) override;
	void CustomizePoolSize(std::vector<uint32_t>& counts) override;

protected:
	bool	m_isVertical;
};