#include "PerObjectUniforms.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "UniformData.h"
#include "Material.h"

bool PerObjectUniforms::Init(const std::shared_ptr<PerObjectUniforms>& pSelf)
{
	if (!ChunkBasedUniforms::Init(pSelf, sizeof(PerObjectVariables)))
		return false;

	return true;
}

std::shared_ptr<PerObjectUniforms> PerObjectUniforms::Create()
{
	std::shared_ptr<PerObjectUniforms> pPerObjectUniforms = std::make_shared<PerObjectUniforms>();
	if (pPerObjectUniforms.get() && pPerObjectUniforms->Init(pPerObjectUniforms))
		return pPerObjectUniforms;
	return nullptr;
}

void PerObjectUniforms::SetModelMatrix(uint32_t index, const Matrix4f& modelMatrix)
{
	m_perObjectVariables[index].prevModelMatrix = m_perObjectVariables[index].modelMatrix;
	m_perObjectVariables[index].modelMatrix = modelMatrix;
	SetChunkDirty(index);
}

void PerObjectUniforms::UpdateDirtyChunkInternal(uint32_t index)
{
	m_perObjectVariables[index].prevMVPN = UniformData::GetInstance()->GetPerFrameUniforms()->GetPrevVPNMatrix() * m_perObjectVariables[index].prevModelMatrix;
	m_perObjectVariables[index].MVPN = UniformData::GetInstance()->GetPerFrameUniforms()->GetVPNMatrix() * m_perObjectVariables[index].modelMatrix;
}

std::vector<UniformVarList> PerObjectUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"PerFrameUniforms",
			{
				{ Mat4Unit, "modelMatrix" },
				{ Mat4Unit, "MVP" },
			}
		}
	};
}

uint32_t PerObjectUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));

	return bindingIndex;
}



