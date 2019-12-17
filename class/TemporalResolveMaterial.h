#pragma once
#include "Material.h"

class RenderPassBase;

class TemporalResolveMaterial : public Material
{
protected:
	bool Init(const std::shared_ptr<TemporalResolveMaterial>& pSelf,
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPassBase>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		const std::vector<UniformVar>& materialUniformVars,
		uint32_t vertexFormat,
		uint32_t vertexFormatInMem,
		uint32_t pingpong);

	void CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout) override;
	void CustomizePoolSize(std::vector<uint32_t>& counts) override;

	void AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, uint32_t pingpong = 0) override;

public:
	static std::shared_ptr<TemporalResolveMaterial> CreateDefaultMaterial(uint32_t pingpong);

public:
	void Dispatch(const std::shared_ptr<CommandBuffer>& pCmdBuf, const Vector3d& groupNum, const Vector3d& groupSize, uint32_t pingpong = 0) override {}
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0, bool overrideVP = false) override
	{
		DrawScreenQuad(pCmdBuf, pFrameBuffer, pingpong, overrideVP);
	}

	void AfterRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, uint32_t pingpong = 0) override;
};