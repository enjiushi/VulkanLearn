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
	Vector2<uint64_t> GetBinaryTreeBits() const { return m_binaryTreeBits; }
	void SetBinaryTreeBits(const Vector2<uint64_t>& binaryTreeBits) { m_binaryTreeBits = binaryTreeBits; }
	Vector3d& GetVertexRef(uint32_t index) { return m_vertices[index]; }

private:
	// Which cube face this tile belongs to
	CubeFace			m_cubeFace;
	// Binary tree bits for U and V direction
	Vector2<uint64_t>	m_binaryTreeBits;
	// 4 vertices
	Vector3d			m_vertices[4];
};