#pragma once
#include "../Base/Base.h"
#include "../Maths/Vector.h"
#include "../Maths/PyramidFrustum.h"
#include "../common/Enums.h"

typedef struct _Triangle
{
	// A triangle consists of a vertex, and 2 edge vectors: edge0 and edge1
	Vector3f	p;
	Vector3f	edge0;
	Vector3f	edge1;
	float		level;	// the sign of this variable gives morphing direction
}Triangle;

class PlanetTile : public SelfRefBase<PlanetTile>
{
public:
	typedef struct _TileInfo
	{
		Vector2d			tileOffset;
		double				tileSize;
		uint32_t			tileLevel;
		double				planetRadius;
		CubeFace			cubeFace;
		std::shared_ptr<PlanetTile>	pParentTile;
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
	CullState GetCullState(uint8_t i) const { return m_cullState[i]; }

	void ProcessFrutumCulling(const PyramidFrustumd& frustum);
	// 4 bits mask order: bottom left, bottom right, top left, top right
	// 0: don't render, 1: render
	// One tile is divided into 4 sub-tiles of the same geometry. Since each one of them might be covered by tiles of the next level,
	// we only render those necessary sub-tiles.
	// Those sub-tiles covered by next level will be marked as 0 in the mask
	void PrepareGeometry(const Vector3d& cameraPosition, double planetRadius, uint32_t level, uint8_t mask, Triangle*& pOutputTriangles);

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
	// Radius of the planet this tile belongs to
	double				m_planetRadius;
	// LOD level this tile resides
	uint32_t			m_tileLevel;
	// 9 vertices of this tile
	// 4 vertices for each sub-tile
	Vector3d			m_realSizeVertices[9];
	// Parent tile and parent sub-tile index
	// Since every PlanetTile consists of 4 sub-tiles, each child PlanetTile overlaps one parent sub-tile
	std::shared_ptr<PlanetTile>	m_pParentTile;
	uint8_t						m_parentSubTileIndex;
	// Culling state for 4 sub-tiles
	CullState					m_cullState[4];

	// Utility vectors
	static Vector3d		m_utilityVector0;
	static Vector3d		m_utilityVector1;
	static Vector3d		m_utilityVector2;
	static Vector3d		m_utilityVector3;
};