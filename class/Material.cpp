#include <mutex>
#include "Material.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/PerFrameResource.h"
#include "../vulkan/PhysicalDevice.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/FrameManager.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/SharedVertexBuffer.h"
#include "../vulkan/SharedIndexBuffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/StagingBufferManager.h"
#include "../vulkan/RenderPass.h"
#include "../vulkan/GraphicPipeline.h"
#include "../vulkan/DescriptorSetLayout.h"
#include "../vulkan/PipelineLayout.h"
#include "../vulkan/ShaderModule.h"
#include "../vulkan/Framebuffer.h"
#include "../vulkan/DescriptorPool.h"
#include "../class/MaterialInstance.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "../class/UniformData.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/Image.h"
#include "../vulkan/SharedIndirectBuffer.h"
#include "RenderWorkManager.h"
#include "../vulkan/GlobalVulkanStates.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../class/PerMaterialIndirectUniforms.h"
#include "../vulkan/Image.h"
#include "GlobalTextures.h"
#include "../vulkan/Image.h"
#include "../vulkan/Texture2DArray.h"
#include "../vulkan/TextureCube.h"
#include "../vulkan/Texture2D.h"
#include "../class/PerMaterialUniforms.h"
#include "RenderPassBase.h"
#include "../vulkan/ComputePipeline.h"

void Material::GeneralInit
(
	const std::vector<VkPushConstantRange>& pushConstsRanges,
	const std::vector<UniformVar>& materialUniformVars,
	bool includeIndirectBuffer
)
{
	uint32_t materialUniformCount = includeIndirectBuffer ? MaterialUniformStorageTypeCount : 1;

	m_materialVariableLayout.resize(materialUniformCount);
	m_materialUniforms.resize(materialUniformCount);


	std::vector<UniformVarList> _materialVariableLayout;

	// Add per material indirect index uniform layout
	if (includeIndirectBuffer)
	{
		m_materialUniforms[PerMaterialIndirectVariableBuffer] = PerMaterialIndirectUniforms::Create();
		m_materialVariableLayout[PerMaterialIndirectVariableBuffer] = m_materialUniforms[PerMaterialIndirectVariableBuffer]->PrepareUniformVarList()[0];
		m_pPerMaterialIndirectUniforms = std::dynamic_pointer_cast<PerMaterialIndirectUniforms>(m_materialUniforms[PerMaterialIndirectVariableBuffer]);
	}

	// Add material variable layout
	m_materialVariableLayout[PerMaterialVariableBuffer] =
	{
		DynamicUniformBuffer,
		"PBR Material Textures Indices",
		{}
	};

	m_materialVariableLayout[PerMaterialVariableBuffer].vars = materialUniformVars;

	// If there's no per material variable, I just simply add a dummy variable, for the sake of simplicity
	if (m_materialVariableLayout[PerMaterialVariableBuffer].vars.size() == 0)
	{
		m_materialVariableLayout[PerMaterialVariableBuffer].vars =
		{
			{
				OneUnit,
				"Dummy"
			}
		};
	}
	m_materialUniforms[PerMaterialVariableBuffer] = PerMaterialUniforms::Create(GetByteSize(m_materialVariableLayout[PerMaterialVariableBuffer].vars));
	m_pPerMaterialUniforms = std::dynamic_pointer_cast<PerMaterialUniforms>(m_materialUniforms[PerMaterialVariableBuffer]);

	// Force per object material variable to be shader storage buffer
	for (auto& var : m_materialVariableLayout)
	{
		if (var.type == DynamicUniformBuffer)
			var.type = DynamicShaderStorageBuffer;
	}

	CustomizeMaterialLayout(m_materialVariableLayout);

	// Build vulkan layout bindings
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for (auto & var : m_materialVariableLayout)
	{
		switch (var.type)
		{
		case DynamicUniformBuffer:
			bindings.push_back
			({
				(uint32_t)bindings.size(),
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
				var.count,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				nullptr
				});

			break;
		case DynamicShaderStorageBuffer:
			bindings.push_back
			({
				(uint32_t)bindings.size(),
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
				var.count,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				nullptr
				});

			break;
		case CombinedSampler:
			bindings.push_back
			({
				(uint32_t)bindings.size(),
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				var.count,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				nullptr
				});
			break;
		case InputAttachment:
			bindings.push_back
			({
				(uint32_t)bindings.size(),
				VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
				var.count,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				nullptr
				});
			break;
		default:
			ASSERTION(false);
			break;
		}
	}
	m_pDescriptorSetLayout = DescriptorSetLayout::Create(GetDevice(), bindings);

	std::vector<std::shared_ptr<DescriptorSetLayout>> descriptorSetLayouts = UniformData::GetInstance()->GetDescriptorSetLayouts();
	descriptorSetLayouts.push_back(m_pDescriptorSetLayout);

	// Create pipeline layout
	m_pPipelineLayout = PipelineLayout::Create(GetDevice(), descriptorSetLayouts, pushConstsRanges);


	// Prepare descriptor pool size according to resources used by this material
	std::vector<uint32_t> counts(VK_DESCRIPTOR_TYPE_RANGE_SIZE);
	for (auto & binding : bindings)
	{
		counts[binding.descriptorType]++;
	}

	CustomizePoolSize(counts);

	std::vector<VkDescriptorPoolSize> descPoolSize;
	for (uint32_t i = 0; i < counts.size(); i++)
	{
		if (counts[i] != 0)
			descPoolSize.push_back({ (VkDescriptorType)i, counts[i] });
	}

	VkDescriptorPoolCreateInfo descPoolInfo = {};
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pPoolSizes = descPoolSize.data();
	descPoolInfo.poolSizeCount = descPoolSize.size();
	descPoolInfo.maxSets = 1 + GetSwapChain()->GetSwapChainImageCount();

	m_pDescriptorPool = DescriptorPool::Create(GetDevice(), descPoolInfo);
	m_pUniformStorageDescriptorSet = m_pDescriptorPool->AllocateDescriptorSet(m_pDescriptorSetLayout);

	m_descriptorSets = UniformData::GetInstance()->GetDescriptorSets();
	m_descriptorSets.push_back(m_pUniformStorageDescriptorSet);

	// Setup descriptor set
	uint32_t bindingIndex = 0;
	for (uint32_t i = 0; i < materialUniformCount; i++)
	{
		bindingIndex = m_materialUniforms[i]->SetupDescriptorSet(m_pUniformStorageDescriptorSet, bindingIndex);
	}

	// Setup cached frame offsets
	m_cachedFrameOffsets = UniformData::GetInstance()->GetCachedFrameOffsets();

	for (uint32_t frameIndex = 0; frameIndex < GetSwapChain()->GetSwapChainImageCount(); frameIndex++)
	{
		std::vector<uint32_t> offsets;
		for (uint32_t i = 0; i < materialUniformCount; i++)
		{
			m_cachedFrameOffsets[frameIndex].push_back(m_materialUniforms[i]->GetFrameOffset() * frameIndex);
		}
	}
}

bool Material::Init
(
	const std::shared_ptr<Material>& pSelf,
	const std::vector<std::wstring>	shaderPaths,
	const std::shared_ptr<RenderPassBase>& pRenderPass,
	const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
	const std::vector<UniformVar>& materialUniformVars,
	uint32_t vertexFormat,
	uint32_t vertexFormatInMem
)
{
	return Init(pSelf, shaderPaths, pRenderPass, pipelineCreateInfo, {}, materialUniformVars, vertexFormat, vertexFormatInMem);
}

bool Material::Init
(
	const std::shared_ptr<Material>& pSelf,
	const std::vector<std::wstring>	shaderPaths,
	const std::shared_ptr<RenderPassBase>& pRenderPass,
	const VkGraphicsPipelineCreateInfo& pipelineCreateInfo,
	const std::vector<VkPushConstantRange>& pushConstsRanges,
	const std::vector<UniformVar>& materialUniformVars,
	uint32_t vertexFormat,
	uint32_t vertexFormatInMem
)
{
	if (!SelfRefBase<Material>::Init(pSelf))
		return false;

	GeneralInit(pushConstsRanges, materialUniformVars, true);

	// Init shaders
	std::vector<std::shared_ptr<ShaderModule>> shaders;
	
	for (uint32_t i = 0; i < (uint32_t)ShaderModule::ShaderTypeCount; i++)
	{
		if (shaderPaths[i] != L"")
			shaders.push_back(ShaderModule::Create(GetDevice(), shaderPaths[i], (ShaderModule::ShaderType)i, "main"));	//FIXME: hard-coded main
	}

	// Create pipeline
	m_pGraphicPipeline = GraphicPipeline::Create(GetDevice(), pipelineCreateInfo, shaders, pRenderPass->GetRenderPass(), m_pPipelineLayout);

	m_pRenderPass = pRenderPass;

	m_pIndirectBuffer = SharedIndirectBuffer::Create(GetDevice(), sizeof(VkDrawIndirectCommand) * MAX_INDIRECT_COUNT);

	m_vertexFormat = vertexFormat;
	m_vertexFormatInMem = vertexFormatInMem;

	return true;
}

bool Material::Init
(
	const std::shared_ptr<Material>& pSelf,
	const std::wstring& shaderPath,
	const VkComputePipelineCreateInfo& pipelineCreateInfo,
	const std::vector<VkPushConstantRange>& pushConstsRanges,
	const std::vector<UniformVar>& materialUniformVars
)
{
	GeneralInit(pushConstsRanges, materialUniformVars, false);

	// Init shader
	std::shared_ptr<ShaderModule> pShader = ShaderModule::Create(GetDevice(), shaderPath, ShaderModule::ShaderType::ShaderTypeCompute, "main");

	// Create pipeline
	m_pComputePipeline = ComputePipeline::Create(GetDevice(), pipelineCreateInfo, pShader, m_pPipelineLayout);

	return true;
}

// This function follows rule of std430
// Could be bugs in it
uint32_t Material::GetByteSize(std::vector<UniformVar>& UBOLayout)
{
	uint32_t offset = 0;
	uint32_t unitCount = 0;
	uint32_t maxAlignUnit = 1;		// Result of max align size, obtained by iterate over the whole layout
	for (auto & var : UBOLayout)
	{
		switch (var.type)
		{
		case OneUnit:
			unitCount += 1;
			if (maxAlignUnit < 1)
				maxAlignUnit = 1;
			break;
		case Vec2Unit:
			unitCount = (unitCount + 1) / 2 * 2;
			unitCount += 2 * var.count;
			if (maxAlignUnit < 2)
				maxAlignUnit = 2;
			break;
		case Vec3Unit:
			unitCount = (unitCount + 3) / 4 * 4;
			unitCount += 4 * var.count;
			if (maxAlignUnit < 4)
				maxAlignUnit = 4;
			break;
		case Vec4Unit:
			unitCount = (unitCount + 3) / 4 * 4;
			unitCount += 4 * var.count;
			if (maxAlignUnit < 4)
				maxAlignUnit = 4;
			break;
		case Mat2x4Unit:
			unitCount = (unitCount + 3) / 4 * 4;
			unitCount += 4 * 2 * var.count;
			if (maxAlignUnit < 4)
				maxAlignUnit = 4;
			break;
		case Mat3Unit:
			unitCount = (unitCount + 3) / 4 * 4;
			unitCount += 4 * 4 * var.count;
			if (maxAlignUnit < 4)
				maxAlignUnit = 4;
			break;
		case Mat4Unit:
			unitCount = (unitCount + 3) / 4 * 4;
			unitCount += 4 * 4 * var.count;
			if (maxAlignUnit < 4)
				maxAlignUnit = 4;
			break;
		default:
			ASSERTION(false);
			break;
		}

		var.offset = offset;
		offset = unitCount * 4;
	}

	// I need to comment here, as I've encountered a wired bug that I just cannot simple get per material uniform data right if object count exceeds over 1
	// After some investigation, I've located this problem:
	// I take alignment into consideration between material variables, however, I didn't take it into consideration between 2 material objects
	// Let's take an example, say a material layout is : vec4, vec2, float, float, float
	// After running this function, offset is: 0-vec4, 4-vec2, 6-float, 7-float, 8-float
	// Things seem okay, but when I have 2 objects with same material, problem occurs:
	// 2 sets of material variable data is not aligned! Since the whole size of a set is 9 unit, and biggest variable is 4 unit!
	// So I have to add padding space to the end of each material variable set, by 3 unit
	// Then, each material variable set is 12 unit big, and is perfectly aligned with each other
	// After doing this, material data of the second object turns back to okay
	uint32_t tailUnitCount = unitCount % maxAlignUnit;
	if (tailUnitCount > 0)
		unitCount = (unitCount / maxAlignUnit + 1) * maxAlignUnit;

	return unitCount * 4;
}

std::shared_ptr<MaterialInstance> Material::CreateMaterialInstance()
{
	std::shared_ptr<MaterialInstance> pMaterialInstance = std::make_shared<MaterialInstance>();

	if (pMaterialInstance.get() && pMaterialInstance->Init(pMaterialInstance))
	{
		std::shared_ptr<PerMaterialUniforms> pPerMaterialUniforms = std::dynamic_pointer_cast<PerMaterialUniforms>(m_materialUniforms[PerMaterialVariableBuffer]);
		pMaterialInstance->m_materialBufferChunkIndex = pPerMaterialUniforms->AllocatePerObjectChunk();
		pMaterialInstance->m_pMaterial = GetSelfSharedPtr();

		m_generatedInstances.push_back(pMaterialInstance);

		return pMaterialInstance;
	}

	return nullptr;
}

uint32_t Material::GetUniformBufferSize() const
{
	return m_materialUniforms[PerMaterialVariableBuffer]->GetBuffer()->GetBufferInfo().size;
}

void Material::SyncBufferData()
{
	for (auto & var : m_materialUniforms)
		if (var != nullptr)
			var->SyncBufferData();
}

void Material::BindPipeline(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	pCmdBuffer->BindPipeline(GetGraphicPipeline());
}

void Material::BindDescriptorSet(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	pCmdBuffer->BindDescriptorSets(GetPipelineLayout(), m_descriptorSets, m_cachedFrameOffsets[FrameMgr()->FrameIndex()]);
}

void Material::SetMaterialTexture(uint32_t index, const std::shared_ptr<Image>& pTexture)
{
	GetDescriptorSet()->UpdateImage(index, pTexture, pTexture->CreateLinearRepeatSampler(), pTexture->CreateDefaultImageView());
}

void Material::BindMeshData(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	if (m_vertexFormatInMem == 0)
		return;

	pCmdBuffer->BindVertexBuffers({ VertexAttribBufferMgr(m_vertexFormatInMem)->GetBuffer() });
	pCmdBuffer->BindIndexBuffer(IndexBufferMgr()->GetBuffer(), VK_INDEX_TYPE_UINT32);
}

void Material::InsertIntoRenderQueue(const VkDrawIndexedIndirectCommand& cmd, uint32_t perObjectIndex, uint32_t perMaterialIndex, uint32_t perMeshIndex, uint32_t perAnimationIndex)
{
	m_pIndirectBuffer->SetIndirectCmd(m_indirectIndex, cmd);

	m_pPerMaterialIndirectUniforms->SetPerObjectIndex(m_indirectIndex, perObjectIndex);
	m_pPerMaterialIndirectUniforms->SetPerMaterialIndex(m_indirectIndex, perMaterialIndex);
	m_pPerMaterialIndirectUniforms->SetPerMeshIndex(m_indirectIndex, perMeshIndex);
	m_pPerMaterialIndirectUniforms->SetPerAnimationIndex(m_indirectIndex, perAnimationIndex);

	m_indirectIndex += 1;
}

void Material::BeforeRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, uint32_t pingpong)
{
	AttachResourceBarriers(pCmdBuf, pingpong);
}

void Material::AfterRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, uint32_t pingpong)
{
}

void Material::OnFrameBegin()
{

}

void Material::OnFrameEnd()
{
	m_indirectIndex = 0;
}

void Material::SetPerObjectIndex(uint32_t indirectIndex, uint32_t perObjectIndex)
{
	m_pPerMaterialIndirectUniforms->SetPerObjectIndex(indirectIndex, perObjectIndex);
}

uint32_t Material::GetPerObjectIndex(uint32_t indirectIndex) const
{
	return m_pPerMaterialIndirectUniforms->GetPerObjectIndex(indirectIndex);
}

void Material::SetPerMaterialIndex(uint32_t indirectIndex, uint32_t perMaterialIndex)
{
	m_pPerMaterialIndirectUniforms->SetPerMaterialIndex(indirectIndex, perMaterialIndex);
}

uint32_t Material::GetPerMaterialIndex(uint32_t indirectIndex) const
{
	return m_pPerMaterialIndirectUniforms->GetPerMaterialIndex(indirectIndex);
}

uint32_t Material::GetParamIndex(const std::string& paramName) const
{
	for (uint32_t i = 0; i < m_materialVariableLayout[PerMaterialVariableBuffer].vars.size(); i++)
		if (m_materialVariableLayout[PerMaterialVariableBuffer].vars[i].name == paramName)
			return i;

	return -1;
}