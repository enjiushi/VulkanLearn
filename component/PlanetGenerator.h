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

	static const uint32_t MAX_LEVEL = 16;
	static const uint32_t TRIANGLE_SCREEN_SIZE = 400;

	enum class CullState
	{
		CULL,			// If a triangle is fully out of a volumn
		CULL_DIVIDE,	// If a triangle intersects with a volumn
		DIVIDE			// If a triangle is fully inside a volumn
	};

	typedef struct _IcoTriangle
	{
		// A triangle consists of 3 vertices, a, b and c
		Vector3f	a;
		Vector3f	b;
		Vector3f	c;
	}IcoTriangle;

public:
	static std::shared_ptr<PlanetGenerator> Create(const std::shared_ptr<PhysicalCamera>& pCamera);

public:
	double GetPlanetRadius() const { return m_planetRadius; }
	void SetPlanetRadius(double radius) { m_planetRadius = radius; }

protected:
	bool Init(const std::shared_ptr<PlanetGenerator>& pSelf, const std::shared_ptr<PhysicalCamera>& pCamera);

protected:
	CullState FrustumCull(const Vector3d& a, const Vector3d& b, const Vector3d& c, double height);
	void SubDivide(uint32_t currentLevel, CullState state, const Vector3d& a, const Vector3d& b, const Vector3d& c, IcoTriangle*& pOutputTriangles);

public:
	void Start() override;
	void OnPreRender() override;

public:
	void ToggleCameraInfoUpdate(bool flag) { m_toggleCameraInfoUpdate = flag; }

private:
	double			m_planetRadius = 1;

	Vector3d		m_icosahedronVertices[20];
	uint32_t		m_icosahedronIndices[20 * 3];

	std::vector<double>				m_distanceLUT;
	std::vector<double>				m_heightLUT;
	std::shared_ptr<MeshRenderer>	m_pMeshRenderer;
	std::shared_ptr<PhysicalCamera>	m_pCamera;

	// Utility variables, to avoid frequent construction and destruction every frame
	Matrix4d		m_utilityTransfrom;
	Vector3d		m_utilityVector0;
	Vector3d		m_utilityVector1;

	// Camera infor in planet local space
	PyramidFrustumd	m_cameraFrustumLocal;
	Vector3d		m_planetSpaceCameraPosition;
	Vector3d		m_lockedPlanetSpaceCameraPosition;

	// Whether to update camera info in planet local space
	// This is used mostly for debugging
	// You can investigate culling result around by setting it to false
	bool			m_toggleCameraInfoUpdate = true;

};