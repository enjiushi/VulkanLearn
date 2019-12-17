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
	if (!ChunkBasedUniforms::Init(pSelf, sizeof(PerObjectVariablesf)))
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

void PerObjectUniforms::SetModelMatrix(uint32_t index, const Matrix4d& modelMatrix)
{
	m_perObjectVariables[index].prevModelMatrix = m_perObjectVariables[index].modelMatrix;
	m_perObjectVariables[index].modelMatrix = modelMatrix;
	SetChunkDirty(index);
}

void PerObjectUniforms::UpdateDirtyChunkInternal(uint32_t index)
{
	m_perObjectVariables[index].prevMVP = UniformData::GetInstance()->GetPerFrameUniforms()->GetPrevVPMatrix().DoublePrecision() * m_perObjectVariables[index].prevModelMatrix;
	m_perObjectVariables[index].MVP = UniformData::GetInstance()->GetPerFrameUniforms()->GetVPMatrix().DoublePrecision() * m_perObjectVariables[index].modelMatrix;

	CONVERT2SINGLE(m_perObjectVariables[index], m_singlePrecisionPerObjectVariables[index], modelMatrix);
	CONVERT2SINGLE(m_perObjectVariables[index], m_singlePrecisionPerObjectVariables[index], MVP);
	CONVERT2SINGLE(m_perObjectVariables[index], m_singlePrecisionPerObjectVariables[index], prevModelMatrix);
	CONVERT2SINGLE(m_perObjectVariables[index], m_singlePrecisionPerObjectVariables[index], prevMVP);
}

std::vector<UniformVarList> PerObjectUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"PerObjectUniforms",
			{
				{ Mat4Unit, "modelMatrix" },
				{ Mat4Unit, "MVP" },
				{ Mat4Unit, "prevModelMatrix" },
				{ Mat4Unit, "prevMVP" },
			}
		}
	};
}

uint32_t PerObjectUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));

	return bindingIndex;
}



