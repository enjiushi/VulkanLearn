#include "PlanetTile.h"
#include "../class/UniformData.h"
#include "../class/GlobalUniforms.h"

bool PlanetTile::Init(const std::shared_ptr<PlanetTile>& pPlanetTile)
{
	if (!SelfRefBase<PlanetTile>::Init(pPlanetTile))
		return false;
	return true;
}

std::shared_ptr<PlanetTile> PlanetTile::Create()
{
	std::shared_ptr<PlanetTile> pPlanetTile = std::make_shared<PlanetTile>();
	if (pPlanetTile != nullptr && pPlanetTile->Init(pPlanetTile))
		return pPlanetTile;
	return nullptr;
}

const Vector3d& PlanetTile::GetNormalizedVertex(uint32_t index)
{
	if (!m_offsetDirty)
		return m_normalizedVertices[index];

	Vector3d* pCubeVertices = &(UniformData::GetInstance()->GetGlobalUniforms()->CubeVertices[0]);
	uint32_t* pCubeIndices = &(UniformData::GetInstance()->GetGlobalUniforms()->CubeIndices[0]);

	// Bottom left and top right vertex
	m_normalizedVertices[0] = pCubeVertices[pCubeIndices[(uint32_t)m_cubeFace * 6 + 0]];
	m_normalizedVertices[3] = pCubeVertices[pCubeIndices[(uint32_t)m_cubeFace * 6 + 5]];

	Vector2d topRightUV = m_tileOffset;
	topRightUV += m_tileSize;

	uint32_t axisU, axisV, axisW;
	double signOnAxisW = 1;
	if (m_cubeFace == CubeFace::BOTTOM || m_cubeFace == CubeFace::TOP)
	{
		axisU = 2; axisV = 0; axisW = 1;	// z and x
	}
	else if (m_cubeFace == CubeFace::FRONT || m_cubeFace == CubeFace::BACK)
	{
		axisU = 0, axisV = 1; axisW = 2;	// x and y
	}
	else
	{
		axisU = 1, axisV = 2; axisW = 0;	// y and z
	}

	if (m_cubeFace == CubeFace::BOTTOM ||
		m_cubeFace == CubeFace::LEFT ||
		m_cubeFace == CubeFace::BACK)
		signOnAxisW = -1;

	// Interpolate and acquire min/max 
	double u0, u1, v0, v1;
	u0 = m_normalizedVertices[0][axisU] * (1.0 - m_tileOffset.x) + m_normalizedVertices[3][axisU] * m_tileOffset.x;
	u1 = m_normalizedVertices[0][axisU] * (1.0 - topRightUV.x) + m_normalizedVertices[3][axisU] * topRightUV.x;
	v0 = m_normalizedVertices[0][axisV] * (1.0 - m_tileOffset.y) + m_normalizedVertices[3][axisV] * m_tileOffset.y;
	v1 = m_normalizedVertices[0][axisV] * (1.0 - topRightUV.y) + m_normalizedVertices[3][axisV] * topRightUV.y;

	static double halfCubeEdgeLength = std::sqrt(3.0) / 3.0 * pCubeVertices[0].Length();

	m_normalizedVertices[0][axisU] = u0; m_normalizedVertices[0][axisV] = v0; m_normalizedVertices[0][axisW] = halfCubeEdgeLength * signOnAxisW;
	m_normalizedVertices[1][axisU] = u1; m_normalizedVertices[1][axisV] = v0; m_normalizedVertices[1][axisW] = m_normalizedVertices[0][axisW];
	m_normalizedVertices[2][axisU] = u0; m_normalizedVertices[2][axisV] = v1; m_normalizedVertices[2][axisW] = m_normalizedVertices[0][axisW];
	m_normalizedVertices[3][axisU] = u1; m_normalizedVertices[3][axisV] = v1; m_normalizedVertices[3][axisW] = m_normalizedVertices[0][axisW];

	m_normalizedVertices[0].Normalize();
	m_normalizedVertices[1].Normalize();
	m_normalizedVertices[2].Normalize();
	m_normalizedVertices[3].Normalize();

	m_offsetDirty = false;

	return m_normalizedVertices[index];
}