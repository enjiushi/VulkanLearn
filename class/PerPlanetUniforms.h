#pragma once

#include "ChunkBasedUniforms.h"

const static uint32_t PLANET_LOD_MAX_LEVEL1 = 32;

template <typename T>
class PerPlanetVariables
{
public:
	/*******************************************************************
	* DESCRIPTION: Planet Rendering Settings
	*
	* X: Planet radius
	* Y: Planet triangle subdivide level
	* Z: Reserved
	* W: Reserved
	*/
	Vector4<T>	PlanetDescriptor0;

	// Planet LOD level distance look up table
	T			PlanetLODDistanceLUT[PLANET_LOD_MAX_LEVEL1];
};

typedef PerPlanetVariables<float> PerPlanetVariablesf;
typedef PerPlanetVariables<double> PerPlanetVariablesd;

class PerPlanetUniforms : public ChunkBasedUniforms
{
protected:
	bool Init(const std::shared_ptr<PerPlanetUniforms>& pSelf);

public:
	static std::shared_ptr<PerPlanetUniforms> Create();

public:
	void SetPlanetRadius(uint32_t index, double radius);
	double GetPlanetRadius(uint32_t index) const { return m_perPlanetVariables[index].PlanetDescriptor0.x; }
	void SetPlanetTriangleSubdivideLevel(uint32_t index, uint32_t level);
	double SetPlanetTriangleSubdivideLevel(uint32_t index) const { return m_perPlanetVariables[index].PlanetDescriptor0.y; }

public:
	std::vector<UniformVarList> PrepareUniformVarList() const override;
	uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const override;

protected:
	void UpdateDirtyChunkInternal(uint32_t index) override {}
	const void* AcquireDataPtr() const override { return &m_singlePrecisionPerPlanetVariables[0]; }
	uint32_t AcquireDataSize() const override { return sizeof(m_singlePrecisionPerPlanetVariables); }

protected:
	PerPlanetVariablesd		m_perPlanetVariables[MAXIMUM_OBJECTS];
	PerPlanetVariablesf		m_singlePrecisionPerPlanetVariables[MAXIMUM_OBJECTS];

	std::vector<uint32_t>	m_dirtyChunks;
};