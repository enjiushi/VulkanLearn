#include "PerMaterialIndirectUniforms.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "UniformData.h"
#include "Material.h"

bool PerMaterialIndirectOffsetUniforms::Init(const std::shared_ptr<PerMaterialIndirectOffsetUniforms>& pSelf)
{
	if (!UniformDataStorage::Init(pSelf, sizeof(m_indirectOffsets), PerFrameDataStorage::ShaderStorage))
		return false;
	return true;
}

std::shared_ptr<PerMaterialIndirectOffsetUniforms> PerMaterialIndirectOffsetUniforms::Create()
{
	std::shared_ptr<PerMaterialIndirectOffsetUniforms> pPerMaterialIndirectOffsetUniforms = std::make_shared<PerMaterialIndirectOffsetUniforms>();
	if (pPerMaterialIndirectOffsetUniforms.get() && pPerMaterialIndirectOffsetUniforms->Init(pPerMaterialIndirectOffsetUniforms))
		return pPerMaterialIndirectOffsetUniforms;
	return nullptr;
}

void PerMaterialIndirectOffsetUniforms::UpdateDirtyChunkInternal(uint32_t index)
{
}

std::vector<UniformVarList> PerMaterialIndirectOffsetUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"PerMaterialIndirectOffset",
			{
				{ OneUnit, "Indirect offset acquired by draw id" }
			}
		}
	};
}

uint32_t PerMaterialIndirectOffsetUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));

	return bindingIndex;
}

bool PerMaterialIndirectUniforms::Init(const std::shared_ptr<PerMaterialIndirectUniforms>& pSelf)
{
	if (!UniformDataStorage::Init(pSelf, sizeof(m_perMaterialIndirectIndex), PerFrameDataStorage::ShaderStorage))
		return false;
	return true;
}

std::shared_ptr<PerMaterialIndirectUniforms> PerMaterialIndirectUniforms::Create()
{
	std::shared_ptr<PerMaterialIndirectUniforms> pPerMaterialIndirectUniforms = std::make_shared<PerMaterialIndirectUniforms>();
	if (pPerMaterialIndirectUniforms.get() && pPerMaterialIndirectUniforms->Init(pPerMaterialIndirectUniforms))
		return pPerMaterialIndirectUniforms;
	return nullptr;
}

void PerMaterialIndirectUniforms::UpdateDirtyChunkInternal(uint32_t index)
{
}

std::vector<UniformVarList> PerMaterialIndirectUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"PerMaterialIndirectIndex",
			{
				{ OneUnit, "Per-object chunk index" },
				{ OneUnit, "Per-material chunk index" },
				{ OneUnit, "Per-mesh chunk index" },
				{ OneUnit, "Per-animation chunk index" },
			}
		}
	};
}

uint32_t PerMaterialIndirectUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));

	return bindingIndex;
}



