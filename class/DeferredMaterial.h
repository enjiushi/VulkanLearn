#pragma once
#include "Material.h"

class RenderPassBase;

class GBufferMaterial : public Material
{
public:
	static std::shared_ptr<GBufferMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf) override;
};

class DeferredShadingMaterial : public Material
{
protected:
	bool Init(const std::shared_ptr<DeferredShadingMaterial>& pSelf,
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPassBase>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		const std::vector<UniformVar>& materialUniformVars,
		uint32_t vertexFormat);

	void CustomizeLayout() override;

public:
	static std::shared_ptr<DeferredShadingMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf) override;
	virtual void OnPassStart() {}
	virtual void OnPassEnd() {}
};