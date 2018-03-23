#pragma once
#include "../Base/BaseComponent.h"
#include "../vulkan/DeviceObjectBase.h"
#include "../class/UniformData.h"
#include "PerMaterialUniforms.h"
#include <map>
#include "Enums.h"

class PipelineLayout;
class GraphicPipeline;
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
	std::vector<VkVertexInputBindingDescription>			vertexBindingsInfo;
	std::vector<VkVertexInputAttributeDescription>			vertexAttributesInfo;
	std::vector<UniformVar>									materialUniformVars;
	uint32_t												vertexFormat;
	bool													isDeferredShadingMaterial = false;
	// FIXME: Render pass is wired thing, as it's used both for pipeline and frame buffer
	// Need to think about where it belongs or belongs to itself
	std::shared_ptr<RenderPass>						pRenderPass;
}SimpleMaterialCreateInfo;

class Material : public SelfRefBase<Material>
{
	static const uint32_t MAX_INDIRECT_COUNT = 2048;

public:
	enum MaterialUniformStorageType
	{
		PerMaterialIndirectVariableBuffer,
		PerMaterialVariableBuffer,
		MaterialUniformStorageTypeCount
	};

public:
	static std::shared_ptr<Material> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	std::shared_ptr<PipelineLayout> GetPipelineLayout() const { return m_pPipelineLayout; }
	std::shared_ptr<GraphicPipeline> GetGraphicPipeline() const { return m_pPipeline; }
	std::shared_ptr<MaterialInstance> CreateMaterialInstance();
	uint32_t GetUniformBufferSize() const;
	std::vector<std::vector<uint32_t>> Material::GetCachedFrameOffsets() const { return m_cachedFrameOffsets; }

	std::shared_ptr<DescriptorSet> GetDescriptorSet() const { return m_pDescriptorSet; }

	void BindPipeline(const std::shared_ptr<CommandBuffer>& pCmdBuffer);
	void BindDescriptorSet(const std::shared_ptr<CommandBuffer>& pCmdBuffer);
	void SetMaterialTexture(uint32_t index, const std::shared_ptr<Image>& pTexture);
	void BindMeshData(const std::shared_ptr<CommandBuffer>& pCmdBuffer);

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

	void SetPerObjectIndex(uint32_t indirectIndex, uint32_t perObjectIndex);
	uint32_t GetPerObjectIndex(uint32_t indirectIndex) const;
	void SetPerMaterialIndex(uint32_t indirectIndex, uint32_t perMaterialIndex);
	uint32_t GetPerMaterialIndex(uint32_t indirectIndex) const;

	void OnPassStart();
	void SyncBufferData();
	void Draw();
	void OnPassEnd();

protected:
	bool Init
	(
		const std::shared_ptr<Material>& pSelf, 
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPass>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		const std::vector<UniformVar>& materialUniformVars,
		uint32_t vertexFormat
	);

	static uint32_t GetByteSize(std::vector<UniformVar>& UBOLayout);
	void InsertIntoRenderQueue(const VkDrawIndexedIndirectCommand& cmd, uint32_t perObjectIndex, uint32_t perMaterialIndex);

protected:
	std::shared_ptr<PipelineLayout>						m_pPipelineLayout;
	std::shared_ptr<GraphicPipeline>					m_pPipeline;

	std::shared_ptr<DescriptorSetLayout>				m_pDescriptorSetLayout;
	std::shared_ptr<DescriptorSet>						m_pDescriptorSet;
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
	friend class MaterialInstance;
};