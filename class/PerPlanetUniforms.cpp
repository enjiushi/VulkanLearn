#include "PerPlanetUniforms.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "UniformData.h"
#include "Material.h"

bool PerPlanetUniforms::Init(const std::shared_ptr<PerPlanetUniforms>& pSelf)
{
	if (!ChunkBasedUniforms::Init(pSelf, sizeof(PerPlanetVariablesf)))
		return false;

	return true;
}

std::shared_ptr<PerPlanetUniforms> PerPlanetUniforms::Create()
{
	std::shared_ptr<PerPlanetUniforms> pPerPlanetUniforms = std::make_shared<PerPlanetUniforms>();
	if (pPerObjectUniforms.get() && pPerObjectUniforms->Init(pPerObjectUniforms))
		return pPerObjectUniforms;
	return nullptr;
}

void PerPlanetUniforms::SetPlanetRadius(uint32_t index, double radius)
{
	m_perPlanetVariables[index].PlanetDescriptor0.x = radius;
	CONVERT2SINGLEVAL(m_perPlanetVariables[index], m_singlePrecisionPerPlanetVariables[index], PlanetDescriptor0.x);

	uint32_t index0 = UniformData::GetInstance()->GetGlobalUniforms()->CubeIndices[1];
	uint32_t index1 = UniformData::GetInstance()->GetGlobalUniforms()->CubeIndices[2];

	double size = (UniformData::GetInstance()->GetGlobalUniforms()->CubeVertices[index0] - UniformData::GetInstance()->GetGlobalUniforms()->CubeVertices[index1]).Length();
	double frac = std::tan(UniformData::GetInstance()->GetGlobalUniforms()->GetMainCameraHorizontalFOV() * UniformData::GetInstance()->GetGlobalUniforms()->GetPlanetTriangleScreenSize() / UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x);
	for (uint32_t i = 0; i < UniformData::GetInstance()->GetGlobalUniforms()->GetMaxPlanetLODLevel() + 1; i++)
	{
		m_perPlanetVariables[index].PlanetLODDistanceLUT[i] = size / frac * radius;
		CONVERT2SINGLEVAL(m_perPlanetVariables[index], m_singlePrecisionPerPlanetVariables[index], PlanetLODDistanceLUT[i]);
		size *= 0.5;
	}

	SetChunkDirty(index);
}

void PerPlanetUniforms::SetPlanetTriangleSubdivideLevel(uint32_t index, uint32_t level)
{
	m_perPlanetVariables[index].PlanetDescriptor0.y = level;
	CONVERT2SINGLEVAL(m_perPlanetVariables[index], m_singlePrecisionPerPlanetVariables[index], PlanetDescriptor0.y);
}

std::vector<UniformVarList> PerPlanetUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"PerPlanetUniforms",
			{
				{ Vec4Unit, "Planet settings" },
				{
					OneUnit,
					"Planet LOD distance look up table",
					PLANET_LOD_MAX_LEVEL1
				},
			}
		}
	};
}

uint32_t PerPlanetUniforms::SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const
{
	pDescriptorSet->UpdateShaderStorageBufferDynamic(bindingIndex++, std::dynamic_pointer_cast<ShaderStorageBuffer>(GetBuffer()));

	return bindingIndex;
}



