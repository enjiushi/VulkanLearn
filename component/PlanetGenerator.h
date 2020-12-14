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
	double ComputeDistanceToTile
	(
		const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d,
		const Vector3d& realSizeA, const Vector3d& realSizeB, const Vector3d& realSizeC, const Vector3d& realSizeD,
		const Vector3d& faceNormal, const double currentTileLength, CubeFace cubeFace
	);

	CullState FrustumCull(const Vector3d& a, const Vector3d& b, const Vector3d& c, double height);
	CullState FrustumCull(const Vector3d& p0, const Vector3d& p1, const Vector3d& p2, const Vector3d& p3, double height);
	bool BackFaceCull(const Vector3d& a, const Vector3d& b, const Vector3d& c);
	// Though we can do it with simply 2 triangle back face cullings, this function could potentially reduce some calculation
	bool BackFaceCull(const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d);
	void SubDivideTriangle(uint32_t currentLevel, CullState state, const Vector3d& a, const Vector3d& b, const Vector3d& c, bool reversed, Triangle*& pOutputTriangles);
	void SubDivideQuad
	(
		uint32_t currentLevel, 
		CubeFace cubeFace,
		double currentTileLength, 
		CullState state, 
		const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d, 
		const Vector3d& faceNormal, 
		Triangle*& pOutputTriangles
	);

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
	std::vector<double>				m_heightLUT;
	std::vector<double>				m_distanceLUT;
	uint32_t						m_maxLODLevel;
	Vector3d*						m_pVertices;
	uint32_t*						m_pIndices;
	bool*							m_pMorphReverse;

	std::shared_ptr<MeshRenderer>	m_pMeshRenderer;
	std::shared_ptr<PhysicalCamera>	m_pCamera;

	// Utility variables, to avoid frequent construction and destruction every frame
	Matrix4d		m_utilityTransfrom;
	Vector3d		m_utilityVector0;
	Vector3d		m_utilityVector1;
	Vector3d		m_utilityVector2;
	Vector3d		m_utilityVector3;
	Vector3d		m_utilityVector4;
	Vector2d		m_utilityVector5;
	Vector2d		m_utilityVector6;

	Vector3d		m_lockedNormalizedPlanetSpaceCameraPosition;
	double			m_lockedPlanetSpaceCameraHeight;
	double			m_squareLockedPlanetSpaceCameraHeight;
	double			m_squarePlanetRadius;

	uint64_t		m_tileMask;
	Vector3d		m_cubeFaceNormals[(uint32_t)CubeFace::COUNT];

	std::vector<std::shared_ptr<PlanetLODLayer>>	m_planetLODLayers;
	Vector3d		m_layerVertices[16];

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