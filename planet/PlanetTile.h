#pragma once
#include "../Base/Base.h"
#include "../Maths/Vector.h"
#include "../common/Enums.h"

class PlanetTile : public SelfRefBase<PlanetTile>
{
public:
	typedef struct _TileInfo
	{
		Vector2d			tileOffset;
		Vector2<uint64_t>	binaryTreeID;
		double				tileSize;
		uint32_t			tileLevel;
		CubeFace			cubeFace;
	}TileInfo;

protected:
	bool Init(const std::shared_ptr<PlanetTile>& pPlanetTile);

public:
	static std::shared_ptr<PlanetTile> Create();

public:
	void InitTile(const TileInfo& tileInfo);
	CubeFace GetCubeFace() const { return m_cubeFace; }
	Vector2d GetTileOffset() const { return m_tileOffset; }
	uint32_t GetTileLevel() const { return m_tileLevel; }
	double GetTileSize() const { return m_tileSize; }
	const Vector3d& GetNormalizedVertex(uint32_t index) { return m_normalizedVertices[index]; }

protected:
	void RegenerateVertices();

private:
	// Which cube face this tile belongs to
	CubeFace			m_cubeFace;
	// Binary tree ID for both u and v axis
	Vector2<uint64_t>	m_binaryTreeID;
	// Tile offset from bottom left corner of a cube face, in normalized coordinate
	Vector2d			m_tileOffset;
	// Tile size in normalized coordinate
	double				m_tileSize;
	// LOD level this tile resides
	uint32_t			m_tileLevel;
	// 4 vertices of this tile, normalized
	Vector3d			m_normalizedVertices[4];
	// Parent tile
	std::weak_ptr<PlanetTile>	m_pParentTile;
};