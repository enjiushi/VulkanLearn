#pragma once
#include "Material.h"

class DeferredGeometryMaterial : public Material
{
public:
	static std::shared_ptr<DeferredGeometryMaterial> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf) override;
};

class DeferredShadingMaterial : public Material
{
protected:
	bool Init(const std::shared_ptr<DeferredShadingMaterial>& pSelf,
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPass>& pRenderPass,
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