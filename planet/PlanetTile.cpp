#include "PlanetTile.h"
#include "../class/UniformData.h"
#include "../class/GlobalUniforms.h"

Vector3d PlanetTile::m_utilityVector0;
Vector3d PlanetTile::m_utilityVector1;
Vector3d PlanetTile::m_utilityVector2;
Vector3d PlanetTile::m_utilityVector3;

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

void PlanetTile::InitTile(const TileInfo& tileInfo)
{
	m_cubeFace = tileInfo.cubeFace;
	m_binaryTreeID = tileInfo.binaryTreeID;
	m_tileOffset = tileInfo.tileOffset;
	m_tileSize = tileInfo.tileSize;
	m_tileLevel = tileInfo.tileLevel;
	
	// Acquire binary tree ID of current tile level
	// 0 for root level
	m_binaryTreeID.x >>= (53 - m_tileLevel);
	m_binaryTreeID.y >>= (53 - m_tileLevel);

	// Add cube face to binary tree id on first 3 bits(up to 6 cube faces, i.e. 110)
	m_binaryTreeID.x += (uint64_t)m_cubeFace << 61;
	m_binaryTreeID.y += (uint64_t)m_cubeFace << 61;

	RegenerateVertices();
}

void PlanetTile::RegenerateVertices()
{
	Vector3d* pCubeVertices = &(UniformData::GetInstance()->GetGlobalUniforms()->CubeVertices[0]);
	uint32_t* pCubeIndices = &(UniformData::GetInstance()->GetGlobalUniforms()->CubeIndices[0]);

	// Bottom left and top right vertex
	m_normalizedVertices[0] = pCubeVertices[pCubeIndices[(uint32_t)m_cubeFace * 6 + 0]];
	m_normalizedVertices[8] = pCubeVertices[pCubeIndices[(uint32_t)m_cubeFace * 6 + 5]];

	Vector2d topRightUV = m_tileOffset;
	topRightUV += m_tileSize;

	uint32_t axisU = (uint32_t)CubeFaceAxisMapping[(uint32_t)m_cubeFace][(uint32_t)NormCoordAxis::U];
	uint32_t axisV = (uint32_t)CubeFaceAxisMapping[(uint32_t)m_cubeFace][(uint32_t)NormCoordAxis::V];
	uint32_t axisW = (uint32_t)CubeFaceAxisMapping[(uint32_t)m_cubeFace][(uint32_t)NormCoordAxis::W];

	double signOnAxisW = 1;
	if (m_cubeFace == CubeFace::BOTTOM ||
		m_cubeFace == CubeFace::LEFT ||
		m_cubeFace == CubeFace::BACK)
		signOnAxisW = -1;

	// Interpolate and acquire min/max 
	double u0, u1, v0, v1;
	u0 = m_normalizedVertices[0][axisU] * (1.0 - m_tileOffset.x) + m_normalizedVertices[8][axisU] * m_tileOffset.x;
	u1 = m_normalizedVertices[0][axisU] * (1.0 - topRightUV.x) + m_normalizedVertices[8][axisU] * topRightUV.x;
	v0 = m_normalizedVertices[0][axisV] * (1.0 - m_tileOffset.y) + m_normalizedVertices[8][axisV] * m_tileOffset.y;
	v1 = m_normalizedVertices[0][axisV] * (1.0 - topRightUV.y) + m_normalizedVertices[8][axisV] * topRightUV.y;

	static double halfCubeEdgeLength = std::sqrt(3.0) / 3.0 * pCubeVertices[0].Length();

	m_normalizedVertices[0][axisU] = u0; m_normalizedVertices[0][axisV] = v0; m_normalizedVertices[0][axisW] = halfCubeEdgeLength * signOnAxisW;
	m_normalizedVertices[1][axisU] = (u0 + u1) * 0.5; m_normalizedVertices[1][axisV] = v0; m_normalizedVertices[1][axisW] = m_normalizedVertices[0][axisW];
	m_normalizedVertices[2][axisU] = u1; m_normalizedVertices[2][axisV] = v0; m_normalizedVertices[2][axisW] = m_normalizedVertices[0][axisW];

	m_normalizedVertices[3][axisU] = u0; m_normalizedVertices[3][axisV] = (v0 + v1) * 0.5; m_normalizedVertices[3][axisW] = m_normalizedVertices[0][axisW];
	m_normalizedVertices[4][axisU] = (u0 + u1) * 0.5; m_normalizedVertices[4][axisV] = (v0 + v1) * 0.5; m_normalizedVertices[4][axisW] = halfCubeEdgeLength * signOnAxisW;
	m_normalizedVertices[5][axisU] = u1; m_normalizedVertices[5][axisV] = (v0 + v1) * 0.5; m_normalizedVertices[5][axisW] = m_normalizedVertices[0][axisW];

	m_normalizedVertices[6][axisU] = u0; m_normalizedVertices[6][axisV] = v1; m_normalizedVertices[6][axisW] = m_normalizedVertices[0][axisW];
	m_normalizedVertices[7][axisU] = (u0 + u1) * 0.5; m_normalizedVertices[7][axisV] = v1; m_normalizedVertices[7][axisW] = m_normalizedVertices[0][axisW];
	m_normalizedVertices[8][axisU] = u1; m_normalizedVertices[8][axisV] = v1; m_normalizedVertices[8][axisW] = m_normalizedVertices[0][axisW];

	m_normalizedVertices[0].Normalize();
	m_normalizedVertices[1].Normalize();
	m_normalizedVertices[2].Normalize();
	m_normalizedVertices[3].Normalize();
	m_normalizedVertices[4].Normalize();
	m_normalizedVertices[5].Normalize();
	m_normalizedVertices[6].Normalize();
	m_normalizedVertices[7].Normalize();
	m_normalizedVertices[8].Normalize();
}

void PlanetTile::PrepareGeometry(const Vector3d& cameraPosition, double planetRadius, uint32_t level, uint8_t mask, Triangle*& pOutputTriangles)
{
	uint32_t i0, i1, i2, i3, offset;
	for (uint32_t i = 0; i < 4; i++)
	{
		// Don't render sub tile that is marked as zero
		// Or children layer will overlap on it
		if ((mask & (1 << i)) == 0)
			continue;

		offset = (i / 2) * 3;
		i0 = offset + 0 + (i % 2);
		i1 = offset + 1 + (i % 2);
		i2 = i0 + 3;
		i3 = i1 + 3;

		m_utilityVector0 = m_normalizedVertices[i0];
		m_utilityVector1 = m_normalizedVertices[i1];
		m_utilityVector2 = m_normalizedVertices[i2];
		m_utilityVector3 = m_normalizedVertices[i3];

		m_utilityVector0 *= planetRadius;
		m_utilityVector1 *= planetRadius;
		m_utilityVector2 *= planetRadius;
		m_utilityVector3 *= planetRadius;

		m_utilityVector0 -= cameraPosition;
		m_utilityVector1 -= cameraPosition;
		m_utilityVector2 -= cameraPosition;
		m_utilityVector3 -= cameraPosition;

		// Triangle abc
		pOutputTriangles->p = m_utilityVector2.SinglePrecision();
		pOutputTriangles->edge0 = m_utilityVector0.SinglePrecision();
		pOutputTriangles->edge1 = m_utilityVector1.SinglePrecision();

		pOutputTriangles->edge0 -= pOutputTriangles->p;
		pOutputTriangles->edge1 -= pOutputTriangles->p;

		// Level + 1 to avoid zero
		pOutputTriangles->level = (float)0 + 1.0f;

		pOutputTriangles++;

		// Triangle cbd
		pOutputTriangles->p = m_utilityVector1.SinglePrecision();
		pOutputTriangles->edge0 = m_utilityVector3.SinglePrecision();
		pOutputTriangles->edge1 = m_utilityVector2.SinglePrecision();

		pOutputTriangles->edge0 -= pOutputTriangles->p;
		pOutputTriangles->edge1 -= pOutputTriangles->p;

		// Minus gives a sign whether to reverse morphing in vertex shader
		pOutputTriangles->level = ((float)0 + 1.0f) * -1.0f;

		pOutputTriangles++;
	}
}