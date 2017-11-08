#pragma once
#include "../Base/BaseComponent.h"
#include "../vulkan/DeviceObjectBase.h"
#include "../class/UniformData.h"
#include "PerMaterialUniforms.h"
#include <map>

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

// More to add
enum MaterialVariableType
{
	DynamicUniformBuffer,
	DynamicShaderStorageBuffer,
	CombinedSampler,
	MaterialVariableTypeCount
};

typedef struct _SimpleMaterialCreateInfo
{
	std::vector<std::wstring>								shaderPaths;
	std::vector<VkVertexInputBindingDescription>			vertexBindingsInfo;
	std::vector<VkVertexInputAttributeDescription>			vertexAttributesInfo;
	uint32_t												maxMaterialInstance = 512;
	std::vector<UniformVarList>								materialVariableLayout;
	// FIXME: Render pass is wired thing, as it's used both for pipeline and frame buffer
	// Need to think about where it belongs or belongs to itself
	std::shared_ptr<RenderPass>						pRenderPass;
}SimpleMaterialCreateInfo;

class Material : public SelfRefBase<Material>
{
public:
	static std::shared_ptr<Material> CreateDefaultMaterial(const SimpleMaterialCreateInfo& simpleMaterialInfo);

public:
	std::shared_ptr<PipelineLayout> GetPipelineLayout() const { return m_pPipelineLayout; }
	std::shared_ptr<GraphicPipeline> GetGraphicPipeline() const { return m_pPipeline; }
	std::vector<std::shared_ptr<DescriptorSetLayout>> GetDescriptorSetLayouts() const { return m_descriptorSetLayouts; }
	std::shared_ptr<MaterialInstance> CreateMaterialInstance();
	uint32_t GetUniformBufferSize(uint32_t bindingIndex) const;
	std::vector<uint32_t> GetFrameOffsets() const;

	template <typename T>
	void SetParameter(uint32_t chunkIndex, uint32_t bindingIndex, uint32_t parameterIndex, T val)
	{
		m_perMaterialUniforms[bindingIndex]->SetParameter(chunkIndex, m_materialVariableLayout[UniformDataStorage::PerObjectMaterialVariable][bindingIndex].vars[parameterIndex].offset, val);
	}

	template <typename T>
	T GetParameter(uint32_t chunkIndex, uint32_t bindingIndex, uint32_t parameterIndex)
	{
		return m_perMaterialUniforms[bindingIndex]->GetParameter<T>(chunkIndex, m_materialVariableLayout[UniformDataStorage::PerObjectMaterialVariable][bindingIndex].vars[parameterIndex].offset);
	}

	void SyncBufferData();

protected:
	bool Init
	(
		const std::shared_ptr<Material>& pSelf, 
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPass>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		uint32_t maxMaterialInstance,
		const std::vector<UniformVarList>& materialVariableLayout
	);

	static uint32_t GetByteSize(std::vector<UniformVar>& UBOLayout);

protected:
	std::shared_ptr<PipelineLayout>						m_pPipelineLayout;
	std::shared_ptr<GraphicPipeline>					m_pPipeline;
	std::vector<std::shared_ptr<DescriptorSetLayout>>	m_descriptorSetLayouts;
	std::shared_ptr<DescriptorPool>						m_pDescriptorPool;
	uint32_t											m_maxMaterialInstance;
	std::vector<std::vector<UniformVarList>>			m_materialVariableLayout;
	std::vector<std::shared_ptr<PerMaterialUniforms>>	m_perMaterialUniforms;
	std::vector<uint32_t>								m_frameOffsets;
	std::vector<std::weak_ptr<MaterialInstance>>		m_generatedInstances;
	friend class MaterialInstance;
};