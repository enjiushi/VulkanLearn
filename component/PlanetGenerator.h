#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../Maths/PyramidFrustum.h"
#include "../class/PerFrameData.h"
#include "../common/Enums.h"
#include "../planet/PlanetTile.h"
#include "../planet/PlanetLODLayer.h"

class MeshRenderer;
class PhysicalCamera;

// The core idea of this class is based on brilliant https://github.com/Illation/PlanetRenderer
class PlanetGenerator : public BaseComponent
{
	DECLARE_CLASS_RTTI(PlanetGenerator);

	enum class CullState
	{
		CULL,			// If a triangle is fully out of a volumn
		CULL_DIVIDE,	// If a triangle intersects with a volumn
		DIVIDE			// If a triangle is fully inside a volumn
	};

public:
	static std::shared_ptr<PlanetGenerator> Create(const std::shared_ptr<PhysicalCamera>& pCamera, float planetRadius);

public:
	double GetPlanetRadius() const { return m_planetRadius; }

protected:
	bool Init(const std::shared_ptr<PlanetGenerator>& pSelf, const std::shared_ptr<PhysicalCamera>& pCamera, float planetRadius);

protected:
	// New planet lod method test function, not named yet
	void NewPlanetLODMethod(Triangle*& pOutputTriangles);

public:
	void Start() override;
	void OnPreRender() override;

public:
	void ToggleCameraInfoUpdate(bool flag) { m_toggleCameraInfoUpdate = flag; }

private:
	double			m_planetRadius = 1;

	// Make a copy here to avoid frequent uniform reading
	std::vector<double>				m_distanceLUT;
	Vector3d*						m_pVertices;
	uint32_t*						m_pIndices;

	std::shared_ptr<MeshRenderer>	m_pMeshRenderer;
	std::shared_ptr<PhysicalCamera>	m_pCamera;

	// Utility variables, to avoid frequent construction and destruction every frame
	Matrix4d		m_utilityTransfrom;
	Vector3d		m_utilityVector0;
	Vector3d		m_utilityVector1;
	Vector3d		m_utilityVector2;
	Vector3d		m_utilityVector3;
	Vector3d		m_utilityVector4;

	Vector3d		m_lockedNormalizedPlanetSpaceCameraPosition;
	double			m_lockedPlanetSpaceCameraHeight;
	double			m_squareLockedPlanetSpaceCameraHeight;
	double			m_squarePlanetRadius;

	uint64_t		m_tileMask;
	Vector3d		m_cubeFaceNormals[(uint32_t)CubeFace::COUNT];

	std::vector<std::shared_ptr<PlanetLODLayer>>	m_planetLODLayers;

	Vector2<uint64_t>	m_prevBinaryCoord;
	CubeFace			m_prevCubeFace;
	uint32_t			m_prevLevel;

	// Camera infor in planet local space
	PyramidFrustumd	m_cameraFrustumLocal;
	Vector3d		m_planetSpaceCameraPosition;
	Vector3d		m_lockedPlanetSpaceCameraPosition;

	// Whether to update camera info in planet local space
	// This is used mostly for debugging
	// You can investigate culling result around by setting it to false
	bool			m_toggleCameraInfoUpdate = true;

	uint32_t		m_chunkIndex;
};