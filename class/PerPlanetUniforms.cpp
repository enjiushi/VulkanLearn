#include "PerPlanetUniforms.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/ShaderStorageBuffer.h"
#include "../vulkan/CommandPool.h"
#include "../vulkan/Queue.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/Image.h"
#include "ResourceBarrierScheduler.h"
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

	// Get indices of 2 diagnal points of a cube face
	uint32_t index0 = UniformData::GetInstance()->GetGlobalUniforms()->CubeIndices[1];
	uint32_t index1 = UniformData::GetInstance()->GetGlobalUniforms()->CubeIndices[2];

	double size = (UniformData::GetInstance()->GetGlobalUniforms()->CubeVertices[index0] - UniformData::GetInstance()->GetGlobalUniforms()->CubeVertices[index1]).Length();
	double frac = std::tan(UniformData::GetInstance()->GetGlobalUniforms()->GetMainCameraHorizontalFOV() * UniformData::GetInstance()->GetGlobalUniforms()->GetPlanetTriangleScreenSize() / UniformData::GetInstance()->GetGlobalUniforms()->GetGameWindowSize().x);
	double patchLength = UniformData::GetInstance()->GetGlobalUniforms()->GetPlanetPatchSubdivideCount();
	for (uint32_t i = 0; i < UniformData::GetInstance()->GetGlobalUniforms()->GetMaxPlanetLODLevel() + 1; i++)
	{
		// We need to take patch length into consideration
		// Patch length gives a patch size measured by meter
		// Larger the patch is, less distance for each lod level
		m_perPlanetVariables[index].PlanetLODDistanceLUT[i] = size / frac * radius / patchLength;
		CONVERT2SINGLEVAL(m_perPlanetVariables[index], m_singlePrecisionPerPlanetVariables[index], PlanetLODDistanceLUT[i]);
		size *= 0.5;
	}

	// Here we calculate max lod level for each planet, roughly
	// We can have cube's edge length through planet radius
	// And then the max subdivide level required for meter level accuracy can be duduced
	// Do remember we have to take patch length into consideration
	// Last "6" is an empirical value given due the non-uniform distributed patch unit caused by curvature distortion of a cube, make it more accurate in the center of a cube face
	double maxLODLevel = std::log(std::sqrt(2.0) * radius) - std::log(patchLength) + 6;
	SetPlanetMAXLODLevel(index, (uint32_t)maxLODLevel);

	SetChunkDirty(index);
}

void PerPlanetUniforms::SetPlanetMAXLODLevel(uint32_t index, uint32_t level)
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

	std::shared_ptr<ResourceBarrierScheduler> pScheduler = ResourceBarrierScheduler::Create();

	// Precompute required data for atmosphere rendering
	// 1. Transmittance
	PreComputeAtmosphereData
	(
		L"../data/shaders/transmittance_gen.comp.spv", 
		pScheduler,
		{ 16, 4, 1 }, 

		{
		},

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
		pScheduler,
		{ 16, 8, 8 }, 
		{
			UniformData::GetInstance()->GetGlobalTextures()->GetTransmittanceTextureDiction(chunkIndex)
		},
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
		pScheduler,
		{ 4, 1, 1 }, 
		{
			UniformData::GetInstance()->GetGlobalTextures()->GetTransmittanceTextureDiction(chunkIndex)
		},
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
			pScheduler,
			{ 16, 8, 8 }, 
			{
				UniformData::GetInstance()->GetGlobalTextures()->GetTransmittanceTextureDiction(chunkIndex),
				UniformData::GetInstance()->GetGlobalTextures()->GetDeltaRayleigh(),
				UniformData::GetInstance()->GetGlobalTextures()->GetDeltaMie(),
				UniformData::GetInstance()->GetGlobalTextures()->GetDeltaMultiScatter(),
				UniformData::GetInstance()->GetGlobalTextures()->GetDeltaIrradiance()
			},
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
			pScheduler,
			{ 4, 1, 1 }, 
			{
				UniformData::GetInstance()->GetGlobalTextures()->GetDeltaRayleigh(),
				UniformData::GetInstance()->GetGlobalTextures()->GetDeltaMie(),
				UniformData::GetInstance()->GetGlobalTextures()->GetDeltaMultiScatter()
			},
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
			pScheduler,
			{ 16, 8, 8 }, 
			{
				UniformData::GetInstance()->GetGlobalTextures()->GetTransmittanceTextureDiction(chunkIndex),
				UniformData::GetInstance()->GetGlobalTextures()->GetDeltaScatterDensity()
			},
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

void PerPlanetUniforms::PreComputeAtmosphereData
(
	const std::wstring& shaderPath,
	const std::shared_ptr<ResourceBarrierScheduler>& pScheduler,
	const Vector3ui& groupSize,
	const std::vector<std::shared_ptr<Image>>& inputTextures,
	const std::vector<std::shared_ptr<Image>>& outputTextures,
	const std::vector<uint8_t>& data,
	uint32_t chunkIndex
)
{
	std::vector<CustomizedComputeMaterial::TextureUnit> textureUnits;
	for (uint32_t i = 0; i < (uint32_t)outputTextures.size(); i++)
	{
		textureUnits.push_back
		(
			{
				i,

				{ { outputTextures[i], outputTextures[i]->CreateLinearClampToEdgeSampler(), outputTextures[i]->CreateDefaultImageView()} },
				VK_IMAGE_ASPECT_COLOR_BIT,
				{ 0, 1, chunkIndex, 1 },
				true,

				CustomizedComputeMaterial::TextureUnit::ALL,

				{
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_IMAGE_LAYOUT_GENERAL,
					VK_ACCESS_SHADER_WRITE_BIT
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
	std::shared_ptr<CommandBuffer> pCommandBuffer = MainThreadCommandPool(PhysicalDevice::QueueFamily::ALL_ROUND)->AllocateCommandBuffer(CommandBuffer::CBLevel::PRIMARY);
	pCommandBuffer->StartPrimaryRecording();

	for (uint32_t i = 0; i < (uint32_t)inputTextures.size(); i++)
	{
		pScheduler->ClaimResourceUsage
		(
			pCommandBuffer,
			inputTextures[i],
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_ACCESS_SHADER_READ_BIT
		);
	}

	pMaterial->BeforeRenderPass(pCommandBuffer, pScheduler);
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



