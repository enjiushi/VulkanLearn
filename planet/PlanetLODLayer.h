#pragma once
#include "../Base/Base.h"
#include "../common/Enums.h"
#include "../Maths/Vector.h"
#include "PlanetTile.h"

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

	void PrepareGeometry(const Vector3d& cameraPosition, double planetRadius, uint32_t level, Triangle*& pOutputTriangles);

private:
	std::shared_ptr<PlanetTile>	m_tiles[(uint32_t)TileAdjacency::COUNT];
	bool						m_tileAvailable[(uint32_t)TileAdjacency::COUNT];
	Vector2d					m_normalizedCoord;
};