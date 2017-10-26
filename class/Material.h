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
class ShaderStorageBuffer;

enum UBOType
{
	OneUnit,
	Vec2Unit,
	Vec3Unit,
	Vec4Unit,
	Mat3Unit,
	Mat4Unit,
	UBOTypeCount
};

// More to add
enum MaterialVariableType
{
	DynamicUniformBuffer,
	DynamicShaderStorageBuffer,
	CombinedSampler,
	MaterialVariableTypeCount
};

enum DescriptorLayout
{
	GlobalVariable,
	PerFrameVariable,
	PerObjectVariable,
	PerObjectMaterialVariable,
	DescriptorLayoutCount
};

typedef struct _UBOVariable
{
	UBOType		type;
	std::string name;
}UBOVariable;

typedef struct _MaterialVariable
{
	MaterialVariableType		type;
	std::string					name;
	std::vector<UBOVariable>	UBOLayout;
}MaterialVariable;

typedef struct _SimpleMaterialCreateInfo
{
	std::vector<std::wstring>								shaderPaths;
	std::vector<VkVertexInputBindingDescription>			vertexBindingsInfo;
	std::vector<VkVertexInputAttributeDescription>			vertexAttributesInfo;
	uint32_t												maxMaterialInstance = 512;
	std::vector<MaterialVariable>							materialVariableLayout;
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

protected:
	bool Init
	(
		const std::shared_ptr<Material>& pSelf, 
		const std::vector<std::wstring>	shaderPaths,
		const std::shared_ptr<RenderPass>& pRenderPass,
		const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
		uint32_t maxMaterialInstance,
		const std::vector<MaterialVariable>& materialVariableLayout
	);

	static uint32_t GetByteSize(const std::vector<UBOVariable>& UBOLayout);

protected:
	std::shared_ptr<PipelineLayout>						m_pPipelineLayout;
	std::shared_ptr<GraphicPipeline>					m_pPipeline;
	std::vector<std::shared_ptr<DescriptorSetLayout>>	m_descriptorSetLayouts;
	std::shared_ptr<DescriptorPool>						m_pDescriptorPool;
	uint32_t											m_maxMaterialInstance;
	std::vector<std::vector<MaterialVariable>>			m_materialVariableLayout;
	std::map<uint32_t, std::shared_ptr<ShaderStorageBuffer>>	m_materialVariableBuffers;

	friend class MaterialInstance;
};