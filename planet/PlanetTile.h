#pragma once
#include "../Base/Base.h"
#include "../Maths/Vector.h"
#include "../common/Enums.h"

class PlanetTile : public SelfRefBase<PlanetTile>
{
protected:
	bool Init(const std::shared_ptr<PlanetTile>& pPlanetTile);

public:
	static std::shared_ptr<PlanetTile> Create();

public:
	CubeFace GetCubeFace() const { return m_cubeFace; }
	void SetCubeFace(CubeFace cubeFace) { m_cubeFace = cubeFace; }
	Vector2d GetTileOffset() const { return m_tileOffset; }
	void SetTileOffset(const Vector2d& tileOffset) { m_tileOffset = tileOffset; }
	uint32_t GetTileLevel() const { return m_tileLevel; }
	void SetTileLevel(uint32_t tileLevel) { m_tileLevel = tileLevel; }
	double GetTileSize() const { return m_tileSize; }
	void SetTileSize(double tileSize) { m_tileSize = tileSize; }

private:
	// Which cube face this tile belongs to
	CubeFace			m_cubeFace;
	// Tile offset from bottom left corner of a cube face, in normalized coordinate
	Vector2d			m_tileOffset;
	// Tile size in normalized coordinate
	double				m_tileSize;
	// LOD level this tile resides
	uint32_t			m_tileLevel;
};