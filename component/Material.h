#pragma once
#include "../Base/BaseComponent.h"
#include "../vulkan/DeviceObjectBase.h"
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

typedef struct _SimpleMaterialCreateInfo
{
	std::vector<std::wstring>								shaderPaths;
	std::vector<std::vector<VkDescriptorSetLayoutBinding>>	descriptorBindingLayout;
	std::vector<VkVertexInputBindingDescription>			vertexBindingsInfo;
	std::vector<VkVertexInputAttributeDescription>			vertexAttributesInfo;
	uint32_t												maxMaterialInstance = 512;
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

protected:
	bool Init
	(
		const std::shared_ptr<Material>& pSelf, 
		const std::vector<std::wstring>	shaderPaths,
		const std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorBindingLayout,
		const std::shared_ptr<RenderPass>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		uint32_t maxMaterialInstance
	);

protected:
	std::shared_ptr<PipelineLayout>						m_pPipelineLayout;
	std::shared_ptr<GraphicPipeline>					m_pPipeline;
	std::vector<std::shared_ptr<DescriptorSetLayout>>	m_descriptorSetLayouts;
	std::shared_ptr<DescriptorPool>						m_pDescriptorPool;
	std::shared_ptr<UniformBuffer>						m_pMaterialVariableBuffer;
	uint32_t											m_maxMaterialInstance;
};