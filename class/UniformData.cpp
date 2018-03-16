#include "UniformData.h"
#include "Material.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/FrameManager.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSetLayout.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/DescriptorPool.h"
#include "GlobalTextures.h"
#include "GBufferInputUniforms.h"

bool UniformData::Init()
{
	if (!Singleton<UniformData>::Init())
		return false;

	m_uniformStorageBuffers.resize(UniformStorageType::PerObjectMaterialVariableBuffer);

	for (uint32_t i = 0; i < UniformStorageType::PerObjectMaterialVariableBuffer; i++)
	{
		switch ((UniformStorageType)i)
		{
		case UniformStorageType::GlobalVariableBuffer: m_uniformStorageBuffers[i] = GlobalUniforms::Create(); break;
		case UniformStorageType::PerFrameVariableBuffer: m_uniformStorageBuffers[i] = PerFrameUniforms::Create(); break;
		case UniformStorageType::PerObjectVariableBuffer: m_uniformStorageBuffers[i] = PerObjectUniforms::Create(); break;
		default:
			break;
		}
	}

	m_uniformTextures.resize(UniformTextureTypeCount);
	for (uint32_t i = 0; i < UniformTextureType::UniformTextureTypeCount; i++)
	{
		switch ((UniformTextureType)i)
		{
		case UniformTextureType::GlobalUniformTextures: m_uniformTextures[i] = GlobalTextures::Create(); break;
		case UniformTextureType::GlobalGBufferInputUniforms: m_uniformTextures[i] = GBufferInputUniforms::Create(); break;
		default:
			break;
		}
	}

	BuildDescriptorSets();

	return true;
}

void UniformData::SyncDataBuffer()
{
	for (auto& var : m_uniformStorageBuffers)
		var->SyncBufferData();
}

std::vector<std::vector<UniformVarList>> UniformData::GenerateUniformVarLayout() const
{
	std::vector<std::vector<UniformVarList>> layout;
	for (uint32_t i = 0; i < UniformStorageType::PerObjectMaterialVariableBuffer; i++)
		layout.push_back(m_uniformStorageBuffers[i]->PrepareUniformVarList());
	return layout;
}

std::vector<uint32_t> UniformData::GetFrameOffsets() const
{
	std::vector<uint32_t> offsets;
	for (uint32_t i = 0; i < UniformStorageType::PerObjectMaterialVariableBuffer; i++)
		offsets.push_back(m_uniformStorageBuffers[i]->GetFrameOffset() * FrameMgr()->FrameIndex());
	return offsets;
}

void UniformData::BuildDescriptorSets()
{
	// Setup global uniform var list, including one global var buffer, one global texture list, one global input attachment list
	std::vector<UniformVarList> globalUniformVars = m_uniformStorageBuffers[UniformStorageType::GlobalVariableBuffer]->PrepareUniformVarList();

	std::vector<UniformVarList> globalTextureVars = m_uniformTextures[UniformTextureType::GlobalUniformTextures]->PrepareUniformVarList();
	globalUniformVars.insert(globalUniformVars.end(), globalTextureVars.begin(), globalTextureVars.end());

	std::vector<UniformVarList> globalInputAttachVars = m_uniformTextures[UniformTextureType::GlobalGBufferInputUniforms]->PrepareUniformVarList();
	globalUniformVars.insert(globalUniformVars.end(), globalInputAttachVars.begin(), globalInputAttachVars.end());

	// Setup per frame uniform var list
	std::vector<UniformVarList> perFrameUniformVars = m_uniformStorageBuffers[UniformStorageType::PerFrameVariableBuffer]->PrepareUniformVarList();

	// Setup per object uniform var list
	std::vector<UniformVarList> perObjectUniformVars = m_uniformStorageBuffers[UniformStorageType::PerObjectVariableBuffer]->PrepareUniformVarList();


	// Put them all together
	std::vector<std::vector<UniformVarList>> uniformVarLists(PerObjectMaterialVariableBufferLocation);
	uniformVarLists[GlobalUniformsLocation]		= globalUniformVars;
	uniformVarLists[PerFrameUniformsLocation]	= perFrameUniformVars;
	uniformVarLists[PerObjectUniformsLocation]	= perObjectUniformVars;

	// Build vulkan layout bindings
	std::vector<std::vector<VkDescriptorSetLayoutBinding>> layoutBindings;
	for (auto & varList : uniformVarLists)
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		for (auto & var : varList)
		{
			switch (var.type)
			{
			case DynamicUniformBuffer:
				bindings.push_back
				({
					(uint32_t)bindings.size(),
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
					1,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					nullptr
					});

				break;
			case DynamicShaderStorageBuffer:
				bindings.push_back
				({
					(uint32_t)bindings.size(),
					VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
					1,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					nullptr
					});

				break;
			case CombinedSampler:
				bindings.push_back
				({
					(uint32_t)bindings.size(),
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					1,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					nullptr
					});
				break;
			case InputAttachment:
				bindings.push_back
				({
					(uint32_t)bindings.size(),
					VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
					1,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					nullptr
					});
				break;
			default:
				ASSERTION(false);
				break;
			}
		}
		m_descriptorSetLayouts.push_back(DescriptorSetLayout::Create(GetDevice(), bindings));
		layoutBindings.push_back(bindings);
	}

	// Prepare descriptor pool size according to resources used by this material
	std::vector<uint32_t> counts(VK_DESCRIPTOR_TYPE_RANGE_SIZE);
	for (auto & bindings : layoutBindings)
	{
		for (auto & binding : bindings)
		{
			counts[binding.descriptorType]++;
		}
	}

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
	descPoolInfo.maxSets = PerObjectMaterialVariableBufferLocation;

	m_pDescriptorPool = DescriptorPool::Create(GetDevice(), descPoolInfo);

	// Allocate descriptor sets according to layouts
	for (auto & layout : m_descriptorSetLayouts)
		m_descriptorSets.push_back(m_pDescriptorPool->AllocateDescriptorSet(layout));



	// Setup descriptor sets data

	// 1. Global descriptor set
	uint32_t bindingSlot = 0;
	bindingSlot = m_uniformStorageBuffers[GlobalVariableBuffer]->SetupDescriptorSet(m_descriptorSets[GlobalUniformsLocation], bindingSlot);
	bindingSlot = m_uniformTextures[GlobalUniformTextures]->SetupDescriptorSet(m_descriptorSets[GlobalUniformsLocation], bindingSlot);
	m_uniformTextures[GlobalGBufferInputUniforms]->SetupDescriptorSet(m_descriptorSets[GlobalUniformsLocation], bindingSlot);

	// 2. Per frame descriptor set
	m_uniformStorageBuffers[PerFrameVariableBuffer]->SetupDescriptorSet(m_descriptorSets[PerFrameUniformsLocation], 0);

	// 3. Per object descriptor set
	m_uniformStorageBuffers[PerObjectVariableBuffer]->SetupDescriptorSet(m_descriptorSets[PerObjectUniformsLocation], 0);

}