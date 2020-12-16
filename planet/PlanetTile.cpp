#include "PlanetTile.h"
#include "../class/UniformData.h"
#include "../class/GlobalUniforms.h"
#include "../common/Util.h"

Vector3d PlanetTile::m_utilityVector0;
Vector3d PlanetTile::m_utilityVector1;
Vector3d PlanetTile::m_utilityVector2;
Vector3d PlanetTile::m_utilityVector3;

bool PlanetTile::Init(const std::shared_ptr<PlanetTile>& pPlanetTile)
{
	if (!SelfRefBase<PlanetTile>::Init(pPlanetTile))
		return false;

	m_binaryTreeID = { 0, 0 };

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
	m_cubeFace		= tileInfo.cubeFace;
	m_tileOffset	= tileInfo.tileOffset;
	m_tileSize		= tileInfo.tileSize;
	m_tileLevel		= tileInfo.tileLevel;
	m_planetRadius	= tileInfo.planetRadius;
	m_pParentTile	= tileInfo.pParentTile;

	m_binaryTreeID = { AcquireBinaryCoord(m_tileOffset.x), AcquireBinaryCoord(m_tileOffset.y) };
	
	// Acquire binary tree ID of current tile level
	// 0 for root level
	m_binaryTreeID.x >>= (fractionBits - m_tileLevel);
	m_binaryTreeID.y >>= (fractionBits - m_tileLevel);

	// Parent sub-tile index: 0, 1, 2, 3
	m_parentSubTileIndex = (m_binaryTreeID.x & 1) + ((m_binaryTreeID.y & 1) << 1);

	RegenerateVertices();
}

void PlanetTile::RegenerateVertices()
{
	Vector3d* pCubeVertices = &(UniformData::GetInstance()->GetGlobalUniforms()->CubeVertices[0]);
	uint32_t* pCubeIndices = &(UniformData::GetInstance()->GetGlobalUniforms()->CubeIndices[0]);

	// Bottom left and top right vertex
	m_realSizeVertices[0] = pCubeVertices[pCubeIndices[(uint32_t)m_cubeFace * 6 + 0]];
	m_realSizeVertices[8] = pCubeVertices[pCubeIndices[(uint32_t)m_cubeFace * 6 + 5]];

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
	u0 = m_realSizeVertices[0][axisU] * (1.0 - m_tileOffset.x) + m_realSizeVertices[8][axisU] * m_tileOffset.x;
	u1 = m_realSizeVertices[0][axisU] * (1.0 - topRightUV.x) + m_realSizeVertices[8][axisU] * topRightUV.x;
	v0 = m_realSizeVertices[0][axisV] * (1.0 - m_tileOffset.y) + m_realSizeVertices[8][axisV] * m_tileOffset.y;
	v1 = m_realSizeVertices[0][axisV] * (1.0 - topRightUV.y) + m_realSizeVertices[8][axisV] * topRightUV.y;

	static double halfCubeEdgeLength = std::sqrt(3.0) / 3.0 * pCubeVertices[0].Length();

	m_realSizeVertices[0][axisU] = u0; m_realSizeVertices[0][axisV] = v0; m_realSizeVertices[0][axisW] = halfCubeEdgeLength * signOnAxisW;
	m_realSizeVertices[1][axisU] = (u0 + u1) * 0.5; m_realSizeVertices[1][axisV] = v0; m_realSizeVertices[1][axisW] = m_realSizeVertices[0][axisW];
	m_realSizeVertices[2][axisU] = u1; m_realSizeVertices[2][axisV] = v0; m_realSizeVertices[2][axisW] = m_realSizeVertices[0][axisW];

	m_realSizeVertices[3][axisU] = u0; m_realSizeVertices[3][axisV] = (v0 + v1) * 0.5; m_realSizeVertices[3][axisW] = m_realSizeVertices[0][axisW];
	m_realSizeVertices[4][axisU] = (u0 + u1) * 0.5; m_realSizeVertices[4][axisV] = (v0 + v1) * 0.5; m_realSizeVertices[4][axisW] = halfCubeEdgeLength * signOnAxisW;
	m_realSizeVertices[5][axisU] = u1; m_realSizeVertices[5][axisV] = (v0 + v1) * 0.5; m_realSizeVertices[5][axisW] = m_realSizeVertices[0][axisW];

	m_realSizeVertices[6][axisU] = u0; m_realSizeVertices[6][axisV] = v1; m_realSizeVertices[6][axisW] = m_realSizeVertices[0][axisW];
	m_realSizeVertices[7][axisU] = (u0 + u1) * 0.5; m_realSizeVertices[7][axisV] = v1; m_realSizeVertices[7][axisW] = m_realSizeVertices[0][axisW];
	m_realSizeVertices[8][axisU] = u1; m_realSizeVertices[8][axisV] = v1; m_realSizeVertices[8][axisW] = m_realSizeVertices[0][axisW];

	m_realSizeVertices[0].Normalize();
	m_realSizeVertices[1].Normalize();
	m_realSizeVertices[2].Normalize();
	m_realSizeVertices[3].Normalize();
	m_realSizeVertices[4].Normalize();
	m_realSizeVertices[5].Normalize();
	m_realSizeVertices[6].Normalize();
	m_realSizeVertices[7].Normalize();
	m_realSizeVertices[8].Normalize();

	m_realSizeVertices[0] *= m_planetRadius;
	m_realSizeVertices[1] *= m_planetRadius;
	m_realSizeVertices[2] *= m_planetRadius;
	m_realSizeVertices[3] *= m_planetRadius;
	m_realSizeVertices[4] *= m_planetRadius;
	m_realSizeVertices[5] *= m_planetRadius;
	m_realSizeVertices[6] *= m_planetRadius;
	m_realSizeVertices[7] *= m_planetRadius;
	m_realSizeVertices[8] *= m_planetRadius;
}

bool PlanetTile::SameTile(const Vector2<uint64_t>& binaryTreeID, CubeFace cubeFace)
{
	return (m_binaryTreeID == binaryTreeID) && (m_cubeFace == cubeFace);
}

void PlanetTile::ProcessFrutumCulling(const PyramidFrustumd& frustum)
{
	m_cullState[0] = m_cullState[1] = m_cullState[2] = m_cullState[3] = CullState::DIVIDE;

	if (m_pParentTile != nullptr)
	{
		CullState parentCullState = m_pParentTile->GetCullState(m_parentSubTileIndex);

		// Early return if parent sub-tile's culling state is cull
		if (parentCullState == CullState::CULL)
		{
			m_cullState[0] = m_cullState[1] = m_cullState[2] = m_cullState[3] = CullState::CULL;
			return;
		}
		// Early return if parent sub-tile's culling state is divide, which means we don't need to do frustum culling
		if (parentCullState == CullState::DIVIDE)
		{
			return;
		}
	}

	// Now we need to do frustum culling since its parent sub-tile is marked as cull and divide

	uint32_t i0, i1, i2, i3, offset;
	for (uint32_t i = 0; i < 4; i++)
	{
		offset = (i / 2) * 3;
		i0 = offset + 0 + (i % 2);
		i1 = offset + 1 + (i % 2);
		i2 = i0 + 3;
		i3 = i1 + 3;

		// We only do left & right frustum face culling, as I don't want to consider altitude into consideration now, or never
		uint32_t outsideCount = 0;
		outsideCount += frustum.planes[PyramidFrustumd::FrustumFace_LEFT].PlaneTest(m_realSizeVertices[i0]) > 0 ? 0 : 1;
		outsideCount += frustum.planes[PyramidFrustumd::FrustumFace_LEFT].PlaneTest(m_realSizeVertices[i1]) > 0 ? 0 : 1;
		outsideCount += frustum.planes[PyramidFrustumd::FrustumFace_LEFT].PlaneTest(m_realSizeVertices[i2]) > 0 ? 0 : 1;
		outsideCount += frustum.planes[PyramidFrustumd::FrustumFace_LEFT].PlaneTest(m_realSizeVertices[i3]) > 0 ? 0 : 1;

		if (outsideCount == 4)
		{
			m_cullState[i] = CullState::CULL;
			continue;
		}
		else if (outsideCount > 0)
		{
			m_cullState[i] = CullState::CULL_DIVIDE;
		}

		outsideCount = 0;
		outsideCount += frustum.planes[PyramidFrustumd::FrustumFace_RIGHT].PlaneTest(m_realSizeVertices[i0]) > 0 ? 0 : 1;
		outsideCount += frustum.planes[PyramidFrustumd::FrustumFace_RIGHT].PlaneTest(m_realSizeVertices[i1]) > 0 ? 0 : 1;
		outsideCount += frustum.planes[PyramidFrustumd::FrustumFace_RIGHT].PlaneTest(m_realSizeVertices[i2]) > 0 ? 0 : 1;
		outsideCount += frustum.planes[PyramidFrustumd::FrustumFace_RIGHT].PlaneTest(m_realSizeVertices[i3]) > 0 ? 0 : 1;

		if (outsideCount == 4)
		{
			m_cullState[i] = CullState::CULL;
			continue;
		}
		else if (outsideCount > 0)
		{
			m_cullState[i] = CullState::CULL_DIVIDE;
		}

		m_utilityVector0 = m_realSizeVertices[i0];
		m_utilityVector1 = m_realSizeVertices[i1];
		m_utilityVector2 = m_realSizeVertices[i2];
		m_utilityVector3 = m_realSizeVertices[i3];
	}
}

void PlanetTile::PrepareGeometry(const Vector3d& cameraPosition, double planetRadius, uint32_t level, uint8_t mask, Triangle*& pOutputTriangles)
{
	if (m_pParentTile != nullptr)
	{
		if (m_pParentTile->GetCullState(m_parentSubTileIndex) == CullState::CULL)
			return;
	}

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

		m_utilityVector0 = m_realSizeVertices[i0];
		m_utilityVector1 = m_realSizeVertices[i1];
		m_utilityVector2 = m_realSizeVertices[i2];
		m_utilityVector3 = m_realSizeVertices[i3];

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