#include "PerPlanetUniforms.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "../vulkan/CommandPool.h"
#include "../vulkan/Queue.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/Image.h"
#include "GlobalTextures.h"
#include "UniformData.h"
#include "Material.h"
#include "CustomizedComputeMaterial.h"

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

	SetChunkDirty(chunkIndex);
	// DO REMEMBER TO SYNC DATA TO GPU BUFFER BEFORE DOING ANYTHING
	// THIS IS NOT NORMAL FRAME RENDERING, I NEED TO DO IT HERE MANUALLY
	UniformData::GetInstance()->SyncDataBuffer();

	std::vector<uint8_t> data;
	data.push_back(*((uint8_t*)&chunkIndex + 0));
	data.push_back(*((uint8_t*)&chunkIndex + 1));
	data.push_back(*((uint8_t*)&chunkIndex + 2));
	data.push_back(*((uint8_t*)&chunkIndex + 3));

	// Precompute required data for atmosphere rendering
	// 1. Transmittance
	PreComputeAtmosphereData
	(
		L"../data/shaders/transmittance_gen.comp.spv", 
		{ 16, 4, 1 }, 
		{
			UniformData::GetInstance()->GetGlobalTextures()->GetTransmittanceTextureDiction(chunkIndex)
		},
		data,
		chunkIndex
	);

	// 2. Single scattering
	PreComputeAtmosphereData
	(
		L"../data/shaders/single_scatter_gen.comp.spv",
		{ 16, 8, 8 },
		{
			UniformData::GetInstance()->GetGlobalTextures()->GetDeltaRayleigh(),
			UniformData::GetInstance()->GetGlobalTextures()->GetDeltaMie(),
			UniformData::GetInstance()->GetGlobalTextures()->GetScatterTextureDiction(chunkIndex)
		},
		data,
		chunkIndex
	);

	// 3. Direct irradiance
	PreComputeAtmosphereData
	(
		L"../data/shaders/direct_irradiance.comp.spv", 
		{ 4, 1, 1 },
		{
			UniformData::GetInstance()->GetGlobalTextures()->GetDeltaIrradiance(),
			UniformData::GetInstance()->GetGlobalTextures()->GetIrradianceTextureDiction(chunkIndex)
		},
		data,
		chunkIndex
	);

	// 4. Multi scatter
	// FIXME: Hard-code 2nd order multi scatter, for test
	for (uint32_t scatterOrder = 2; scatterOrder <= 4; scatterOrder++)
	{
		data.push_back(*((uint8_t*)&scatterOrder + 0));
		data.push_back(*((uint8_t*)&scatterOrder + 1));
		data.push_back(*((uint8_t*)&scatterOrder + 2));
		data.push_back(*((uint8_t*)&scatterOrder + 3));

		// 4.1 Delta scatter density
		PreComputeAtmosphereData
		(
			L"../data/shaders/delta_rayleigh_mie_gen.comp.spv",
			{ 16, 8, 8 },
			{
				UniformData::GetInstance()->GetGlobalTextures()->GetDeltaScatterDensity()
			},
			data,
			chunkIndex
		);

		// 4.2 Indirect irradiance
		uint32_t irradianceOrder = scatterOrder - 1;
		data.erase(data.begin() + 4, data.end());
		data.push_back(*((uint8_t*)&irradianceOrder + 0));
		data.push_back(*((uint8_t*)&irradianceOrder + 1));
		data.push_back(*((uint8_t*)&irradianceOrder + 2));
		data.push_back(*((uint8_t*)&irradianceOrder + 3));
		PreComputeAtmosphereData
		(
			L"../data/shaders/indirect_irradiance_gen.comp.spv",
			{ 4, 1, 1 },
			{
				UniformData::GetInstance()->GetGlobalTextures()->GetDeltaIrradiance(),
				UniformData::GetInstance()->GetGlobalTextures()->GetIrradianceTextureDiction(chunkIndex)
			},
			data,
			chunkIndex
		);

		// 4.3 Multi scatter
		data.erase(data.begin() + 4, data.end());
		data.push_back(*((uint8_t*)&scatterOrder + 0));
		data.push_back(*((uint8_t*)&scatterOrder + 1));
		data.push_back(*((uint8_t*)&scatterOrder + 2));
		data.push_back(*((uint8_t*)&scatterOrder + 3));
		PreComputeAtmosphereData
		(
			L"../data/shaders/multi_scatter_gen.comp.spv",
			{ 16, 8, 8 },
			{
				UniformData::GetInstance()->GetGlobalTextures()->GetDeltaMultiScatter(),
				UniformData::GetInstance()->GetGlobalTextures()->GetScatterTextureDiction(chunkIndex)
			},
			data,
			chunkIndex
		);

		data.erase(data.begin() + 4, data.end());
	}

	return chunkIndex;
}

void PerPlanetUniforms::UpdateDirtyChunkInternal(uint32_t index)
{
}

void PerPlanetUniforms::PreComputeAtmosphereData(const std::wstring& shaderPath, const Vector3ui& groupSize, const std::vector<std::shared_ptr<Image>>& textures, const std::vector<uint8_t>& data, uint32_t chunkIndex)
{
	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	for (uint32_t i = 0; i < (uint32_t)textures.size(); i++)
	{
		textureUnits.push_back
		(
			{
				i,

				{ {textures[i], textures[i]->CreateLinearClampToEdgeSampler(), textures[i]->CreateDefaultImageView()} },
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, chunkIndex, 1 },
				true,

				CustomizedComputeMaterial::TextureUnit::ALL,

				{
					{
						true,
						VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						textures[i]->GetImageInfo().initialLayout,
						VK_ACCESS_SHADER_READ_BIT,

						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						VK_IMAGE_LAYOUT_GENERAL,
						VK_ACCESS_SHADER_WRITE_BIT
					},
					{
						true,
						VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						VK_IMAGE_LAYOUT_GENERAL,
						VK_ACCESS_SHADER_WRITE_BIT,

						VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						textures[i]->GetImageInfo().initialLayout,
						VK_ACCESS_SHADER_READ_BIT
					}
				}
			}
		);
	}

	CustomizedComputeMaterial::Variables vars =
	{
		shaderPath,
		groupSize,
		textureUnits,
		data
	};
	std::shared_ptr<Material> pMaterial = CustomizedComputeMaterial::CreateMaterial(vars);

	// Recording
	std::shared_ptr<CommandBuffer> pCommandBuffer = MainThreadGraphicPool()->AllocateCommandBuffer(CommandBuffer::CBLevel::PRIMARY);
	pCommandBuffer->StartPrimaryRecording();

	pMaterial->BeforeRenderPass(pCommandBuffer);
	pMaterial->Dispatch(pCommandBuffer);
	pMaterial->AfterRenderPass(pCommandBuffer);

	pCommandBuffer->EndPrimaryRecording();

	// Do the job
	GlobalObjects()->GetQueue(PhysicalDevice::QueueFamily::ALL_ROUND)->SubmitCommandBuffer(pCommandBuffer, nullptr, true);
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



