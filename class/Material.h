#pragma once
#include "../Base/BaseComponent.h"
#include "../vulkan/DeviceObjectBase.h"
#include "../class/UniformData.h"
#include "PerMaterialUniforms.h"
#include "FrameBufferDiction.h"
#include <map>
#include "../common/Enums.h"
#include "../Maths/Vector3.h"

class PipelineLayout;
class GraphicPipeline;
class ComputePipeline;
class DescriptorSetLayout;
class DescriptorSet;
class ShaderModule;
class RenderPass;
class MaterialInstance;
class DescriptorPool;
class UniformBuffer;
class ShaderStorageBuffer;
class CommandBuffer;
class Image;
class SharedIndirectBuffer;
class PerMaterialIndirectUniforms;
class FrameBuffer;
class RenderPassBase;

// More to add
enum MaterialVariableType
{
	DynamicUniformBuffer,
	DynamicShaderStorageBuffer,
	CombinedSampler,
	InputAttachment,
	MaterialVariableTypeCount
};

typedef struct _SimpleMaterialCreateInfo
{
	std::vector<std::wstring>								shaderPaths;
	std::vector<UniformVar>									materialUniformVars;
	uint32_t												vertexFormat;
	uint32_t												vertexFormatInMem;
	uint32_t												subpassIndex = 0;
	FrameBufferDiction::FrameBufferType						frameBufferType;
	std::shared_ptr<RenderPassBase>							pRenderPass = nullptr;
	bool													isTransparent = false;
	bool													depthTestEnable = true;
	bool													depthWriteEnable = true;
}SimpleMaterialCreateInfo;

class Material : public SelfRefBase<Material>
{
	static const uint32_t MAX_INDIRECT_COUNT = 2048;

public:
	enum MaterialUniformStorageType
	{
		PerMaterialVariableBuffer,
		PerMaterialIndirectVariableBuffer,
		MaterialUniformStorageTypeCount
	};

public:
	std::shared_ptr<RenderPassBase> GetRenderPass() const { return m_pRenderPass; }
	std::shared_ptr<PipelineLayout> GetPipelineLayout() const { return m_pPipelineLayout; }
	std::shared_ptr<GraphicPipeline> GetGraphicPipeline() const { return m_pGraphicPipeline; }
	std::shared_ptr<ComputePipeline> GetComputePipeline() const { return m_pComputePipeline; }
	std::shared_ptr<MaterialInstance> CreateMaterialInstance();
	uint32_t GetUniformBufferSize() const;
	std::vector<std::vector<uint32_t>> Material::GetCachedFrameOffsets() const { return m_cachedFrameOffsets; }
	uint32_t GetVertexFormat() const { return m_vertexFormat; }
	uint32_t GetVertexFormatInMem() const { return m_vertexFormatInMem; }

	std::shared_ptr<DescriptorSet> GetDescriptorSet() const { return m_pUniformStorageDescriptorSet; }

	virtual void BindPipeline(const std::shared_ptr<CommandBuffer>& pCmdBuffer);
	virtual void BindDescriptorSet(const std::shared_ptr<CommandBuffer>& pCmdBuffer);
	virtual void SetMaterialTexture(uint32_t index, const std::shared_ptr<Image>& pTexture);
	virtual void BindMeshData(const std::shared_ptr<CommandBuffer>& pCmdBuffer);

	virtual void AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, uint32_t pingpong = 0) {}

	template <typename T>
	void SetParameter(uint32_t chunkIndex, uint32_t parameterIndex, T val)
	{
		m_pPerMaterialUniforms->SetParameter(chunkIndex, m_materialVariableLayout[PerMaterialVariableBuffer].vars[parameterIndex].offset, val);
	}

	template <typename T>
	T GetParameter(uint32_t chunkIndex, uint32_t parameterIndex)
	{
		return m_pPerMaterialUniforms->GetParameter<T>(chunkIndex, m_materialVariableLayout[PerMaterialVariableBuffer].vars[parameterIndex].offset);
	}

	template <typename T>
	void SetParameter(uint32_t chunkIndex, const std::string& paramName, T val)
	{
		m_pPerMaterialUniforms->SetParameter(chunkIndex, m_materialVariableLayout[PerMaterialVariableBuffer].vars[GetParamIndex(paramName)].offset, val);
	}

	template <typename T>
	T GetParameter(uint32_t chunkIndex, const std::string& paramName)
	{
		return m_pPerMaterialUniforms->GetParameter<T>(chunkIndex, m_materialVariableLayout[PerMaterialVariableBuffer].vars[GetParamIndex(paramName)].offset);
	}

	void SetPerObjectIndex(uint32_t indirectIndex, uint32_t perObjectIndex);
	uint32_t GetPerObjectIndex(uint32_t indirectIndex) const;
	void SetPerMaterialIndex(uint32_t indirectIndex, uint32_t perMaterialIndex);
	uint32_t GetPerMaterialIndex(uint32_t indirectIndex) const;
	uint32_t GetParamIndex(const std::string& paramName) const;

	virtual void SyncBufferData();

	virtual void BeforeRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, uint32_t pingpong = 0);
	virtual void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0) = 0;
	virtual void Dispatch(const std::shared_ptr<CommandBuffer>& pCmdBuf, const Vector3f& groupNum, const Vector3f& groupSize, uint32_t pingpong = 0) = 0;
	virtual void AfterRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, uint32_t pingpong = 0);

	virtual void OnFrameBegin();
	virtual void OnFrameEnd();

protected:
	void GeneralInit
	(
		const std::vector<VkPushConstantRange>& pushConstsRanges,
		const std::vector<UniformVar>& materialUniformVars
	);

	bool Init
	(
		const std::shared_ptr<Material>& pSelf, 
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPassBase>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		const std::vector<UniformVar>& materialUniformVars,
		uint32_t vertexFormat,
		uint32_t vertexFormatInMem
	);

	bool Init
	(
		const std::shared_ptr<Material>& pSelf,
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPassBase>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		const std::vector<VkPushConstantRange>& pushConstsRanges,
		const std::vector<UniformVar>& materialUniformVars,
		uint32_t vertexFormat,
		uint32_t vertexFormatInMem
	);

	bool Init
	(
		const std::shared_ptr<Material>& pSelf,
		const std::wstring& shaderPath,
		const VkComputePipelineCreateInfo& pipelineCreateInfo,
		const std::vector<VkPushConstantRange>& pushConstsRanges,
		const std::vector<UniformVar>& materialUniformVars
	);

	virtual void CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout) {}
	virtual void CustomizePoolSize(std::vector<uint32_t>& counts) {}

	static uint32_t GetByteSize(std::vector<UniformVar>& UBOLayout);
	void InsertIntoRenderQueue(const VkDrawIndexedIndirectCommand& cmd, uint32_t perObjectIndex, uint32_t perMaterialIndex, uint32_t perMeshIndex, uint32_t perAnimationIndex);

protected:
	std::shared_ptr<RenderPassBase>						m_pRenderPass;

	std::shared_ptr<PipelineLayout>						m_pPipelineLayout;
	std::shared_ptr<GraphicPipeline>					m_pGraphicPipeline;
	std::shared_ptr<ComputePipeline>					m_pComputePipeline;

	std::shared_ptr<DescriptorSetLayout>				m_pDescriptorSetLayout;
	std::shared_ptr<DescriptorSet>						m_pUniformStorageDescriptorSet;
	std::shared_ptr<DescriptorPool>						m_pDescriptorPool;
	std::vector<std::shared_ptr<DescriptorSet>>			m_descriptorSets;	// Including descriptor sets from uniform data, and "m_pDescriptorSet" of this class

	std::vector<UniformVarList>							m_materialVariableLayout;

	std::vector<std::shared_ptr<UniformDataStorage>>	m_materialUniforms;
	std::vector<std::vector<uint32_t>>					m_cachedFrameOffsets;

	std::shared_ptr<PerMaterialIndirectUniforms>		m_pPerMaterialIndirectUniforms;
	std::shared_ptr<PerMaterialUniforms>				m_pPerMaterialUniforms;

	std::vector<std::weak_ptr<MaterialInstance>>		m_generatedInstances;
	std::shared_ptr<SharedIndirectBuffer>				m_pIndirectBuffer;
	uint32_t											m_indirectIndex = 0;
	uint32_t											m_vertexFormat;
	uint32_t											m_vertexFormatInMem;
	friend class MaterialInstance;
};