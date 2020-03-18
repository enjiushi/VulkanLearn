#pragma once
#include "Material.h"

class RenderPassBase;
class CommandBuffer;

class SSAOMaterial : public Material
{
protected:
	bool Init(const std::shared_ptr<SSAOMaterial>& pSelf,
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPassBase>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		const std::vector<UniformVar>& materialUniformVars,
		uint32_t vertexFormat,
		uint32_t vertexFormatInMem);

	void CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout) override;
	void CustomizePoolSize(std::vector<uint32_t>& counts) override;

	void CustomizeCommandBuffer(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0) override;
	void AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, BarrierInsertionPoint barrierInsertionPoint, uint32_t pingpong = 0) override;

public:
	static std::shared_ptr<SSAOMaterial> CreateDefaultMaterial();

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0, bool overrideVP = false) override
	{
		DrawScreenQuad(pCmdBuf, pFrameBuffer, pingpong, overrideVP);
	}

protected:
	float	m_blueNoiseTexIndex;
};