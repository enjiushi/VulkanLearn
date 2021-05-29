#include "PlanetLODLayer.h"
#include "../common/Util.h"
#include <cmath>

bool PlanetLODLayer::Init(const std::shared_ptr<PlanetLODLayer>& pPlanetLODLayer)
{
	if (!SelfRefBase<PlanetLODLayer>::Init(pPlanetLODLayer))
		return false;

	for (uint32_t i = 0; i < (uint32_t)TileAdjacency::COUNT; i++)
	{
		m_tiles[i] = PlanetTile::Create();
		m_tiles[i + (uint32_t)TileAdjacency::COUNT] = PlanetTile::Create();
		m_tileAvailable[i] = false;
	}

	// Define adjacent cube tile folding info
	m_cubeTileFolding[(uint32_t)CubeFace::RIGHT][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::FRONT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, input.x };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::RIGHT][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::BOTTOM,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 1.0 - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::RIGHT][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::TOP,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 1.0 - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::RIGHT][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::BACK,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, input.x };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};

	m_cubeTileFolding[(uint32_t)CubeFace::LEFT][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::FRONT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, 1.0 - input.x - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::LEFT][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::TOP,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 0 };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::LEFT][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::BOTTOM,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 0 };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::LEFT][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::BACK,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, 1.0 - input.x - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
		}
	};

	m_cubeTileFolding[(uint32_t)CubeFace::TOP][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::RIGHT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, input.x };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::TOP][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::BACK,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 1.0 - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::TOP][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::FRONT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 1.0 - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::TOP][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::LEFT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, input.x };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};

	m_cubeTileFolding[(uint32_t)CubeFace::BOTTOM][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::RIGHT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, 1.0 - input.x - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BOTTOM][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::FRONT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 0 };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BOTTOM][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::BACK,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 0 };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BOTTOM][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::LEFT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, 1.0 - input.x - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
		}
	};

	m_cubeTileFolding[(uint32_t)CubeFace::FRONT][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::TOP,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, input.x };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::FRONT][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::LEFT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 1.0 - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::FRONT][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::RIGHT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 1.0 - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::FRONT][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::BOTTOM,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, input.x };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};

	m_cubeTileFolding[(uint32_t)CubeFace::BACK][(uint32_t)TileAdjacency::TOP] =
	{
		CubeFace::TOP,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 0, 1.0 - input.x - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BACK][(uint32_t)TileAdjacency::LEFT] =
	{
		CubeFace::RIGHT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ input.y, 0 };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BACK][(uint32_t)TileAdjacency::RIGHT] =
	{
		CubeFace::LEFT,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - input.y - tileSize, 0 };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 1)
				+ (((mask >> 1) & 1) << 3)
				+ (((mask >> 2) & 1) << 0)
				+ (((mask >> 3) & 1) << 2);
		}
	};
	m_cubeTileFolding[(uint32_t)CubeFace::BACK][(uint32_t)TileAdjacency::BOTTOM] =
	{
		CubeFace::BOTTOM,
		[](const Vector2d& input, const double& tileSize)
		{
			return Vector2d{ 1.0 - tileSize, 1.0 - input.x - tileSize };
		},
		[](uint8_t mask)
		{
			return (((mask >> 0) & 1) << 2)
				+ (((mask >> 1) & 1) << 0)
				+ (((mask >> 2) & 1) << 3)
				+ (((mask >> 3) & 1) << 1);
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

void PlanetLODLayer::BuildupLayer(CubeFace cubeFace, uint32_t level, double planetRadius, const Vector2<uint64_t>& binaryCoord, const std::shared_ptr<PlanetLODLayer>& pParentLayer)
{
	m_cubeFace = cubeFace;

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
	Vector2<uint64_t> binaryTreeID;
	uint8_t prevTilePingpongIndex = (m_tilePingpongIndex + 1) % 2;

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
			adjacentTileNormCoord = m_cubeTileFolding[(uint32_t)cubeFace][(uint32_t)mappingAdjacency].transformNormCoordToAdjacentTile(adjacentTileNormCoord, currentLayerTileSize);

		std::shared_ptr<PlanetTile> pParentTile;
		if (pParentLayer != nullptr)
		{
			// Acquire parent tile
			// Offset last bit of binary coordinate by -1, 0, or 1
			// We could have -1, 0, 1, or 2
			// -1 and 2 are not parent layer's center tile, but its adjacent one, in corresponding axis
			// -1==>left/bottom, 0,1==>middle, 2==>right/top
			auto getAdjacentIndex = [](int32_t val)
			{
				if (val < 0)
					return 0;
				else if (val > 1)
					return 2;
				else
					return 1;
			};
			adjU = (m_currentLayerBinaryCoord.x & 1) + (adjU - 1);
			adjV = (m_currentLayerBinaryCoord.y & 1) + (adjV - 1);
			adjU = getAdjacentIndex(adjU);
			adjV = getAdjacentIndex(adjV);
			
			pParentTile = pParentLayer->GetTile((TileAdjacency)(adjU + 3 * adjV));
		}

		binaryTreeID = { AcquireBinaryCoord(adjacentTileNormCoord.x), AcquireBinaryCoord(adjacentTileNormCoord.y) };
		binaryTreeID.x >>= (fractionBits - level);
		binaryTreeID.y >>= (fractionBits - level);

		bool reuse = false;
		uint8_t currIndex = prevTilePingpongIndex * (uint32_t)TileAdjacency::COUNT + j;	// Storing new tiles into prev pingpong
		for (uint32_t i = 0; i < (uint32_t)TileAdjacency::COUNT; i++)
		{
			if (m_tiles[m_tilePingpongIndex * (uint32_t)TileAdjacency::COUNT + i]->SameTile(binaryTreeID, _cubeFace))
			{
				std::swap(m_tiles[currIndex], m_tiles[m_tilePingpongIndex * (uint32_t)TileAdjacency::COUNT + i]);
				reuse = true;
				break;
			}
		}

		if (!reuse)
			m_tiles[currIndex]->InitTile({ adjacentTileNormCoord, currentLayerTileSize, level, planetRadius, _cubeFace, pParentTile });
	}

	// Set prev pingpong to current
	m_tilePingpongIndex = prevTilePingpongIndex;
}

void PlanetLODLayer::ProcessFrutumCulling(const PyramidFrustumd& frustum)
{
	for (uint32_t j = 0; j < (uint32_t)TileAdjacency::COUNT; j++)
	{
		if (!m_tileAvailable[j])
			continue;

		m_tiles[m_tilePingpongIndex * (uint32_t)TileAdjacency::COUNT + j]->ProcessFrutumCulling(frustum);
	}
}

void PlanetLODLayer::PrepareGeometry(const Vector3d& cameraPosition, double planetRadius, uint32_t level, const std::shared_ptr<PlanetLODLayer>& pChildLayer, Triangle*& pOutputTriangles)
{
	memset(m_adjacentTileMasks, -1, sizeof(m_adjacentTileMasks));
	if (pChildLayer != nullptr)
	{
		uint8_t lastBitU = pChildLayer->m_currentLayerBinaryCoord.x & 1;
		uint8_t lastBitV = pChildLayer->m_currentLayerBinaryCoord.y & 1;

		// Acquire adjacent tile in axis U and V which needs to mask out some sub-tiles
		uint8_t adjU = (lastBitU & 1) * 2 + 3;
		uint8_t adjV = (lastBitV & 1) * 6 + 1;

		// 10 is the mask 1010(order: 0-1-0-1) for left adjacent tile
		// 10 >> 1 is the mask 0101(order: 1-0-1-0) for right adjacent tile
		m_adjacentTileMasks[adjU] = (10 >> lastBitU);

		// 12 is the mask 1100(order: 0-0-1-1) for bottom adjacent tile
		// 12 >> 2 is the mask 0011(order: 1-1-0-0) for top adjacent tile
		m_adjacentTileMasks[adjV] = (12 >> (lastBitV * 2));

		// Invert bits so that 0 means masked out sub-tiles
		m_adjacentTileMasks[adjU] = ~m_adjacentTileMasks[adjU];
		m_adjacentTileMasks[adjV] = ~m_adjacentTileMasks[adjV];

		// Mask 1000(order: 0-0-0-1) for bottom left tile
		// Mask 0100(order: 0-0-1-0) for bottom right tile
		// Mask 0010(order: 0-1-0-0) for top left tile
		// Mask 0001(order: 1-0-0-0) for top right tile
		// Each one of them is right shifted 1 bit
		uint8_t topRightMask = (1 << 3);
		// Tile0->binaryCoord(0,0), tile2->binaryCoord(1,0), tile6->binaryCoord(0,1), tile8->binaryCoord(1,1)
		// lastBitU * 2 + 6 + lastBitV * 2: Acquire corner tile index(0, 2, 6, 8)
		uint8_t cornerTileIndex = lastBitU * 2 + lastBitV * 6;
		// lastBitU + lastBitV * 2: Right shift 0, 1, 2, 3, for tile0, tile2, tile6, tile8 respectively
		m_adjacentTileMasks[cornerTileIndex] = (topRightMask >> (lastBitU + lastBitV * 2));
		// Invert
		m_adjacentTileMasks[cornerTileIndex] = ~m_adjacentTileMasks[cornerTileIndex];

		// Don't forget mask of middle tile
		// Since it's always overlapped by children tiles, we mask it out
		m_adjacentTileMasks[(uint32_t)TileAdjacency::MIDDLE] = 0;

		// Handle different face case
		if (m_adjacentTileDifferentFace[adjU])
			m_adjacentTileMasks[adjU] = m_cubeTileFolding[(uint32_t)m_cubeFace][adjU].transfromMaskToAdjacentTile(m_adjacentTileMasks[adjU]);
		if (m_adjacentTileDifferentFace[adjV])
			m_adjacentTileMasks[adjV] = m_cubeTileFolding[(uint32_t)m_cubeFace][adjV].transfromMaskToAdjacentTile(m_adjacentTileMasks[adjV]);
		if (m_adjacentTileDifferentFace[cornerTileIndex] && m_tileAvailable[cornerTileIndex])
		{
			if (m_adjacentTileDifferentFace[adjU])
				m_adjacentTileMasks[cornerTileIndex] = m_cubeTileFolding[(uint32_t)m_cubeFace][adjU].transfromMaskToAdjacentTile(m_adjacentTileMasks[cornerTileIndex]);
			if (m_adjacentTileDifferentFace[adjV])
				m_adjacentTileMasks[cornerTileIndex] = m_cubeTileFolding[(uint32_t)m_cubeFace][adjV].transfromMaskToAdjacentTile(m_adjacentTileMasks[cornerTileIndex]);
		}
	}

	for (uint32_t j = 0; j < (uint32_t)TileAdjacency::COUNT; j++)
	{
		if (!m_tileAvailable[j])
			continue;

		m_tiles[m_tilePingpongIndex * (uint32_t)TileAdjacency::COUNT + j]->PrepareGeometry(cameraPosition, planetRadius, level, m_adjacentTileMasks[j], pOutputTriangles);
	}
}