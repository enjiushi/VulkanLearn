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
		uint32_t vertexFormat,
		uint32_t vertexFormatInMem);

	void CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout) override;
	void CustomizePoolSize(std::vector<uint32_t>& counts) override;

public:
	static std::shared_ptr<PostProcessingMaterial> CreateDefaultMaterial();

public:
	void Dispatch(const std::shared_ptr<CommandBuffer>& pCmdBuf, const Vector3f& groupNum, const Vector3f& groupSize, uint32_t pingpong = 0) override {}
	void AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, uint32_t pingpong = 0) override;
};