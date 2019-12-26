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
	m_perObjectVariables[index].prevMV =  m_perObjectVariables[index].MV;
	m_perObjectVariables[index].MV = UniformData::GetInstance()->GetPerFrameUniforms()->GetViewMatrix() * modelMatrix;
	SetChunkDirty(index);
}

void PerObjectUniforms::UpdateDirtyChunkInternal(uint32_t index)
{
	m_perObjectVariables[index].MVP = UniformData::GetInstance()->GetGlobalUniforms()->GetProjectionMatrix();
	m_perObjectVariables[index].prevMVP = m_perObjectVariables[index].MVP;
	m_perObjectVariables[index].MV_Rotation_P = m_perObjectVariables[index].MVP;
	m_perObjectVariables[index].prevMV_Rotation_P = m_perObjectVariables[index].MVP;


	m_perObjectVariables[index].MVP *= m_perObjectVariables[index].MV;
	m_perObjectVariables[index].prevMVP *= m_perObjectVariables[index].prevMV;


	Matrix4d temp = m_perObjectVariables[index].MV;
	temp.c30 = temp.c31 = temp.c32 = 0;
	m_perObjectVariables[index].MV_Rotation_P *= temp;

	temp = m_perObjectVariables[index].prevMV;
	temp.c30 = temp.c31 = temp.c32 = 0;
	m_perObjectVariables[index].prevMV_Rotation_P *= temp;

	CONVERT2SINGLE(m_perObjectVariables[index], m_singlePrecisionPerObjectVariables[index], MV);
	CONVERT2SINGLE(m_perObjectVariables[index], m_singlePrecisionPerObjectVariables[index], MVP);
	CONVERT2SINGLE(m_perObjectVariables[index], m_singlePrecisionPerObjectVariables[index], MV_Rotation_P);
	CONVERT2SINGLE(m_perObjectVariables[index], m_singlePrecisionPerObjectVariables[index], prevMV);
	CONVERT2SINGLE(m_perObjectVariables[index], m_singlePrecisionPerObjectVariables[index], prevMVP);
	CONVERT2SINGLE(m_perObjectVariables[index], m_singlePrecisionPerObjectVariables[index], prevMV_Rotation_P);
}

std::vector<UniformVarList> PerObjectUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"PerObjectUniforms",
			{
				{ Mat4Unit, "MV" },
				{ Mat4Unit, "MVP" },
				{ Mat4Unit, "MV_Rotation_P" },
				{ Mat4Unit, "prevMV" },
				{ Mat4Unit, "prevMVP" },
				{ Mat4Unit, "prevMV_Rotation_P" },
			}
		}
	};
}

uint32_t PerObjectUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));

	return bindingIndex;
}



