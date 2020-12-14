#pragma once
#include "../Base/Base.h"
#include "../common/Enums.h"
#include "../Maths/Vector.h"
#include "PlanetTile.h"
#include <functional>

class PlanetLODLayer : public SelfRefBase<PlanetLODLayer>
{
protected:
	bool Init(const std::shared_ptr<PlanetLODLayer>& pPlanetLODLayer);

public:
	static std::shared_ptr<PlanetLODLayer> Create();

public:
	std::shared_ptr<PlanetTile> GetTile(TileAdjacency tileAdj) const
	{ 
		if (m_tileAvailable[(uint32_t)tileAdj])
			return m_tiles[(uint32_t)tileAdj]; 
		return nullptr;
	}

	bool IsTileAvailable(TileAdjacency tileAdj) const { return m_tileAvailable[(uint32_t)tileAdj]; }
	void SetTileAvailable(TileAdjacency tileAdj, bool available) { m_tileAvailable[(uint32_t)tileAdj] = available; }
	const Vector2d& GetNormalizedCoord() const { return m_normalizedCoord; }
	void SetNormalizedCoord(const Vector2d& normalizedCoord) { m_normalizedCoord = normalizedCoord; }

	void BuildupLayer(CubeFace cubeFace, uint32_t level, const Vector2<uint64_t>& binaryCoord, const std::shared_ptr<PlanetLODLayer>& pParentLayer);
	void PrepareGeometry(const Vector3d& cameraPosition, double planetRadius, uint32_t level, const std::shared_ptr<PlanetLODLayer>& pChildLayer, Triangle*& pOutputTriangles);

private:
	std::shared_ptr<PlanetTile>	m_tiles[(uint32_t)TileAdjacency::COUNT];
	// Indicates which adjacent tile is available
	bool						m_tileAvailable[(uint32_t)TileAdjacency::COUNT];
	// Indicates which adjacent tile is located on a different cube face
	bool						m_adjacentTileDifferentFace[(uint32_t)TileAdjacency::COUNT];
	// Normalized coordinate of current layer, i.e. the offset coordinate of the bottom left corner of the center tile in this layer
	Vector2d					m_normalizedCoord;
	// Current layer's binary coordinate, indicating a binary tree position on both axis Uand V
	Vector2<uint64_t>			m_currentLayerBinaryCoord;
	// Adjacent tile masks to store which sub-tile should be rendered
	// NOTE: This mask list will be generated per-frame, so it's only used as a utility array. Don't trust the value inside
	uint8_t						m_adjacentTileMasks[(uint32_t)TileAdjacency::COUNT];

	// To handle adjacent tile folding due to cube nature
	typedef struct _TileAdjInfo
	{
		// Adjacent cube face
		CubeFace	cubeFace;
		// The axis that adjacent cube face folds towards, in normlized coordinate axis
		NormCoordAxis	foldingAxis;
		// The folding direction
		Sign		foldingDirection;
		// Transform normalized coordinate at edge to adjacent tile
		std::function<Vector2d(const Vector2d&, const double&)> transform;
	}TileAdjInfo;
	TileAdjInfo		m_cubeTileFolding[(uint32_t)CubeFace::COUNT][(uint32_t)TileAdjacency::COUNT];
};