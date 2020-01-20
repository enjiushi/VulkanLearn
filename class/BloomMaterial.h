#pragma once
#include "Material.h"

class RenderPassBase;

class BloomMaterial : public Material
{
public:
	enum BloomPass
	{
		BloomPass_Prefilter,
		BloomPass_DownSampleBox13,
		BloomPass_UpSampleTent,
		BloomPass_Count
	};

protected:
	bool Init(const std::shared_ptr<BloomMaterial>& pSelf,
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPassBase>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		const std::vector<VkPushConstantRange>& pushConstsRanges,
		const std::vector<UniformVar>& materialUniformVars,
		uint32_t vertexFormat,
		uint32_t vertexFormatInMem);

	void CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout) override;
	void CustomizePoolSize(std::vector<uint32_t>& counts) override;

	void CustomizeSecondaryCmd(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0) override;
	void AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, uint32_t pingpong = 0) override;

public:
	static std::shared_ptr<BloomMaterial> CreateDefaultMaterial(BloomPass bloomPass, uint32_t iterIndex);

public:
	void Dispatch(const std::shared_ptr<CommandBuffer>& pCmdBuf, const Vector3ui& groupSize, uint32_t pingpong = 0) override {}
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0, bool overrideVP = false) override
	{
		DrawScreenQuad(pCmdBuf, pFrameBuffer, pingpong, overrideVP);
	}

private:
	BloomPass	m_bloomPass;
	uint32_t	m_iterIndex;
};