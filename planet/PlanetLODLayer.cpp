#include "PlanetLODLayer.h"
#include "../common/Util.h"

bool PlanetLODLayer::Init(const std::shared_ptr<PlanetLODLayer>& pPlanetLODLayer)
{
	if (!SelfRefBase<PlanetLODLayer>::Init(pPlanetLODLayer))
		return false;

	for (uint32_t i = 0; i < (uint32_t)TileAdjacency::COUNT; i++)
	{
		m_tiles[i] = PlanetTile::Create();
		m_tileAvailable[i] = false;
	}

	// Define adjacent cube tile folding info
	m_cubeTileFolding[(uint32_t)CubeFace::RIGHT][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::FRONT,
		NormCoordAxis::U,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, input.x };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::RIGHT][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::BOTTOM,
		NormCoordAxis::V,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 1.0 - tileSize };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::RIGHT][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::TOP,
		NormCoordAxis::V,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 1.0 - tileSize };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::RIGHT][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::BACK,
		NormCoordAxis::U,
		Sign::NEGATIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, input.x };
		}
	};

	m_cubeTileFolding[(uint32_t)CubeFace::LEFT][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::FRONT,
		NormCoordAxis::U,
		Sign::NEGATIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, 1.0 - input.x - tileSize };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::LEFT][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::TOP,
		NormCoordAxis::V,
		Sign::NEGATIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 0 };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::LEFT][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::BOTTOM,
		NormCoordAxis::V,
		Sign::NEGATIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 0 };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::LEFT][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::BACK,
		NormCoordAxis::U,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, 1.0 - input.x - tileSize };
		}
	};

	m_cubeTileFolding[(uint32_t)CubeFace::TOP][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::RIGHT,
		NormCoordAxis::U,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, input.x };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::TOP][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::BACK,
		NormCoordAxis::V,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 1.0 - tileSize };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::TOP][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::FRONT,
		NormCoordAxis::V,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 1.0 - tileSize };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::TOP][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::LEFT,
		NormCoordAxis::U,
		Sign::NEGATIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, input.x };
		}
	};

	m_cubeTileFolding[(uint32_t)CubeFace::BOTTOM][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::RIGHT,
		NormCoordAxis::U,
		Sign::NEGATIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, 1.0 - input.x - tileSize };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BOTTOM][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::FRONT,
		NormCoordAxis::V,
		Sign::NEGATIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 0 };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BOTTOM][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::BACK,
		NormCoordAxis::V,
		Sign::NEGATIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 0 };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BOTTOM][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::LEFT,
		NormCoordAxis::U,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, 1.0 - input.x - tileSize };
		}
	};

	m_cubeTileFolding[(uint32_t)CubeFace::FRONT][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::TOP,
		NormCoordAxis::U,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, input.x };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::FRONT][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::LEFT,
		NormCoordAxis::V,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 1.0 - tileSize };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::FRONT][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::RIGHT,
		NormCoordAxis::V,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 1.0 - tileSize };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::FRONT][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::BOTTOM,
		NormCoordAxis::U,
		Sign::NEGATIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, input.x };
		}
	};

	m_cubeTileFolding[(uint32_t)CubeFace::BACK][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::TOP,
		NormCoordAxis::U,
		Sign::NEGATIVE,[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, 1.0 - input.x - tileSize };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BACK][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::RIGHT,
		NormCoordAxis::V,
		Sign::NEGATIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 0 };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BACK][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::LEFT,
		NormCoordAxis::V,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 0 };
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BACK][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::BOTTOM,
		NormCoordAxis::U,
		Sign::POSITIVE,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, 1.0 - input.x - tileSize };
		}
	};

	return true;
}

std::shared_ptr<PlanetLODLayer> PlanetLODLayer::Create()
{
	std::shared_ptr<PlanetLODLayer> pPlanetLODLayer = std::make_shared<PlanetLODLayer>();
	if (pPlanetLODLayer != nullptr && pPlanetLODLayer->Init(pPlanetLODLayer))
		return pPlanetLODLayer;
	return nullptr;
}

void PlanetLODLayer::BuildupLayer(CubeFace cubeFace, uint32_t level, const Vector2<uint64_t>& binaryCoord, const std::shared_ptr<PlanetLODLayer>& pParentLayer)
{
	memset(m_tileAvailable, true, sizeof(bool) * (uint32_t)TileAdjacency::COUNT);
	memset(m_adjacentTileDifferentFace, false, sizeof(bool) * (uint32_t)TileAdjacency::COUNT);
	double currentLayerTileSize = std::pow(0.5, (double)level);
	uint64_t maxBinary = (1 << level) - 1;

	if (level == 0)
	{
		m_tileAvailable[(uint32_t)TileAdjacency::TOP_LEFT]
			= m_tileAvailable[(uint32_t)TileAdjacency::TOP_RIGHT]
			= m_tileAvailable[(uint32_t)TileAdjacency::BOTTOM_LEFT]
			= m_tileAvailable[(uint32_t)TileAdjacency::BOTTOM_RIGHT]
			= false;

		m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::TOP]
			= m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::LEFT]
			= m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::RIGHT]
			= m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::BOTTOM]
			= true;
	}
	else
	{
		// Binary coordinate of current layer
		m_currentLayerBinaryCoord.x = (binaryCoord.x >> (fractionBits - level));
		m_currentLayerBinaryCoord.y = (binaryCoord.y >> (fractionBits - level));

		// Normalized coordinate of current layer
		m_normalizedCoord = { 0, 0 };
		if (pParentLayer != nullptr)
			m_normalizedCoord = pParentLayer->GetNormalizedCoord();

		m_normalizedCoord.x += (currentLayerTileSize * (m_currentLayerBinaryCoord.x & 1));
		m_normalizedCoord.y += (currentLayerTileSize * (m_currentLayerBinaryCoord.y & 1));

		// Handle edge case
		if (m_currentLayerBinaryCoord.x == 0)
		{
			m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::TOP_LEFT]
				= m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::LEFT]
				= m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::BOTTOM_LEFT]
				= true;
		}

		if (m_currentLayerBinaryCoord.x == maxBinary)
		{
			m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::TOP_RIGHT]
				= m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::RIGHT]
				= m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::BOTTOM_RIGHT]
				= true;
		}

		if (m_currentLayerBinaryCoord.y == 0)
		{
			m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::BOTTOM_LEFT]
				= m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::BOTTOM]
				= m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::BOTTOM_RIGHT]
				= true;
		}

		if (m_currentLayerBinaryCoord.y == maxBinary)
		{
			m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::TOP_LEFT]
				= m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::TOP]
				= m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::TOP_RIGHT]
				= true;
		}

		// Handle corner case
		if (m_currentLayerBinaryCoord.x == 0 && m_currentLayerBinaryCoord.y == 0)
			m_tileAvailable[(uint32_t)TileAdjacency::BOTTOM_LEFT] = false;
		else if (m_currentLayerBinaryCoord.x == 0 && m_currentLayerBinaryCoord.y == maxBinary)
			m_tileAvailable[(uint32_t)TileAdjacency::TOP_LEFT] = false;
		else if (m_currentLayerBinaryCoord.x == maxBinary && m_currentLayerBinaryCoord.y == 0)
			m_tileAvailable[(uint32_t)TileAdjacency::BOTTOM_RIGHT] = false;
		else if (m_currentLayerBinaryCoord.x == maxBinary && m_currentLayerBinaryCoord.y == maxBinary)
			m_tileAvailable[(uint32_t)TileAdjacency::TOP_RIGHT] = false;
	}

	Vector2d adjacentTileNormCoord{ 0, 0 };
	int32_t adjU, adjV;
	for (uint32_t j = 0; j < (uint32_t)TileAdjacency::COUNT; j++)
	{
		if (!m_tileAvailable[j])
			continue;

		TileAdjacency mappingAdjacency = (TileAdjacency)j;
		adjacentTileNormCoord = m_normalizedCoord;

		// Transform adjacent enums into 2 dimensions
		adjU = j % 3;
		adjV = j / 3;

		// If a tile is located on a border in axis U, and its adjacent tile is outside of current face,
		// then we remove the other axis from corner case.
		// E.g. bottom left, bottom right, top left, top right will be transformed into left, right, left, right
		// The reason to do this is that we only have transforms of different face in 4 adjacent directions, left, right, bottom, top
		if ((m_currentLayerBinaryCoord.x == 0 && adjU == 0) || (m_currentLayerBinaryCoord.x == maxBinary && adjU == 2))
			mappingAdjacency = (TileAdjacency)(adjU + 3);
		else
			adjacentTileNormCoord.x += ((adjU - 1) * currentLayerTileSize);

		// Same applies axis V
		if ((m_currentLayerBinaryCoord.y == 0 && adjV == 0) || (m_currentLayerBinaryCoord.y == maxBinary && adjV == 2))
			mappingAdjacency = (TileAdjacency)(adjV * 3 + 1);
		else
			adjacentTileNormCoord.y += ((adjV - 1) * currentLayerTileSize);

		CubeFace _cubeFace = m_adjacentTileDifferentFace[j] ? m_cubeTileFolding[(uint32_t)cubeFace][(uint32_t)mappingAdjacency].cubeFace : cubeFace;

		if (m_adjacentTileDifferentFace[j])
			adjacentTileNormCoord = m_cubeTileFolding[(uint32_t)cubeFace][(uint32_t)mappingAdjacency].transform(adjacentTileNormCoord, currentLayerTileSize);

		m_tiles[j]->InitTile({ adjacentTileNormCoord, m_currentLayerBinaryCoord, currentLayerTileSize, level, _cubeFace });
	}
}

void PlanetLODLayer::PrepareGeometry(const Vector3d& cameraPosition, double planetRadius, uint32_t level, Triangle*& pOutputTriangles)
{
	for (uint32_t j = 0; j < (uint32_t)TileAdjacency::COUNT; j++)
	{
		if (!m_tileAvailable[j])
			continue;

		m_tiles[j]->PrepareGeometry(cameraPosition, planetRadius, level, pOutputTriangles);
	}
}