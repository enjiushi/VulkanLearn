#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../Maths/PyramidFrustum.h"
#include "../class/PerFrameData.h"

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

	typedef struct _Triangle
	{
		// A triangle consists of a vertex, and 2 edge vectors: edge0 and edge1
		Vector3f	p;
		Vector3f	edge0;
		Vector3f	edge1;
		float		level;	// the sign of this variable gives morphing direction
	}Triangle;

public:
	static std::shared_ptr<PlanetGenerator> Create(const std::shared_ptr<PhysicalCamera>& pCamera, float planetRadius);

public:
	double GetPlanetRadius() const { return m_planetRadius; }

protected:
	bool Init(const std::shared_ptr<PlanetGenerator>& pSelf, const std::shared_ptr<PhysicalCamera>& pCamera, float planetRadius);

protected:
	CullState FrustumCull(const Vector3d& a, const Vector3d& b, const Vector3d& c, double height);
	CullState FrustumCull(const Vector3d& p0, const Vector3d& p1, const Vector3d& p2, const Vector3d& p3, double height);
	void SubDivideIcoTriangle(uint32_t currentLevel, CullState state, const Vector3d& a, const Vector3d& b, const Vector3d& c, bool reversed, Triangle*& pOutputTriangles);

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

	// Camera infor in planet local space
	PyramidFrustumd	m_cameraFrustumLocal;
	Vector3d		m_planetSpaceCameraPosition;
	Vector3d		m_lockedPlanetSpaceCameraPosition;

	// Whether to update camera info in planet local space
	// This is used mostly for debugging
	// You can investigate culling result around by setting it to false
	bool			m_toggleCameraInfoUpdate = true;

};