#include "PerPlanetUniforms.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "GlobalTextures.h"
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
	if (pPerPlanetUniforms.get() && pPerPlanetUniforms->Init(pPerPlanetUniforms))
		return pPerPlanetUniforms;
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

uint32_t PerPlanetUniforms::AllocatePlanetChunk()
{
	uint32_t chunkIndex = AllocatePerObjectChunk();

	AtmosphereParameters<double> atmosphereParameters = 
	{
		{ 1.474000, 1.850400, 1.911980, 0.004675 },
		{ 6360.000000, 6420.000000, 0.800000, -0.207912 },
		{ 0.005802, 0.013558, 0.033100, 0 },
		{ 0.003996, 0.003996, 0.003996, 0 },
		{ 0.004440, 0.004440, 0.004440, 0 },
		{ 0.000650, 0.001881, 0.000085, 0 },
		{ 0.100000, 0.100000, 0.100000, 0 },
		{
			{ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0, 0, 0,
			  0.000000, 1.000000, -0.125000, 0.000000, 0.000000, 0, 0, 0 }
		},
		{
			{ 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0, 0, 0,
			  0.000000, 1.000000, -0.833333, 0.000000, 0.000000, 0, 0, 0 }
		},
		{
			{ 25.000000, 0.000000, 0.000000, 0.066667, -0.666667, 0, 0, 0,
			  0.000000, 0.000000, 0.000000, -0.066667, 2.666667, 0, 0, 0 }
		}
	};

	m_perPlanetVariables[chunkIndex] =
	{
		atmosphereParameters,
		{ 6378000, 32, 1, 0 },
		{ }
	};

	CONVERT2SINGLE(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.solarIrradiance);
	CONVERT2SINGLE(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.variables);
	CONVERT2SINGLE(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.rayleighScattering);
	CONVERT2SINGLE(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.mieScattering);
	CONVERT2SINGLE(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.mieExtinction);
	CONVERT2SINGLE(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.absorptionExtinction);
	CONVERT2SINGLE(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.groundAlbedo);

	for (uint32_t i = 0; i < 2; i++)
	{
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.rayleighDensity.layers[i].width);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.rayleighDensity.layers[i].expTerm);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.rayleighDensity.layers[i].expScale);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.rayleighDensity.layers[i].linearTerm);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.rayleighDensity.layers[i].constantTerm);
	}
	for (uint32_t i = 0; i < 2; i++)
	{
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.mieDensity.layers[i].width);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.mieDensity.layers[i].expTerm);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.mieDensity.layers[i].expScale);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.mieDensity.layers[i].linearTerm);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.mieDensity.layers[i].constantTerm);
	}
	for (uint32_t i = 0; i < 2; i++)
	{
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.absorptionDensity.layers[i].width);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.absorptionDensity.layers[i].expTerm);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.absorptionDensity.layers[i].expScale);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.absorptionDensity.layers[i].linearTerm);
		CONVERT2SINGLEVAL(m_perPlanetVariables[chunkIndex], m_singlePrecisionPerPlanetVariables[chunkIndex], AtmosphereParameters.absorptionDensity.layers[i].constantTerm);
	}

	return chunkIndex;
}

std::vector<UniformVarList> PerPlanetUniforms::PrepareUniformVarList() const
{
	return
	{
		{
			DynamicShaderStorageBuffer,
			"PerPlanetUniforms",
			{
				{ Vec4Unit, "Solar irradiance and sun angular radius" },
				{ Vec4Unit, "Variables" },
				{ Vec4Unit, "Rayleigh scattering" },
				{ Vec4Unit, "Mie scattering" },
				{ Vec4Unit, "Mie extinction" },
				{ Vec4Unit, "Absorption extinction" },
				{ Vec4Unit, "Average ground albedo" },
				{ Vec4Unit, "Rayleigh density profile0 data" },
				{ Vec4Unit, "Rayleigh density profile0 data" },
				{ Vec4Unit, "Rayleigh density profile1 data" },
				{ Vec4Unit, "Rayleigh density profile1 data" },
				{ Vec4Unit, "Mie density profile0 data" },
				{ Vec4Unit, "Mie density profile0 data" },
				{ Vec4Unit, "Mie density profile1 data" },
				{ Vec4Unit, "Mie density profile1 data" },
				{ Vec4Unit, "Absorption density profile0 data" },
				{ Vec4Unit, "Absorption density profile0 data" },
				{ Vec4Unit, "Absorption density profile1 data" },
				{ Vec4Unit, "Absorption density profile1 data" },
				{
					OneUnit,
					"Planet LOD distance look up table",
					PLANET_LOD_MAX_LEVEL
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


