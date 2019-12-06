#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../Maths/PyramidFrustum.h"
#include "../class/PerFrameData.h"

class MeshRenderer;
class PhysicalCamera;

class PlanetGenerator : public BaseComponent
{
	DECLARE_CLASS_RTTI(PlanetGenerator);

	static const uint32_t MAX_LEVEL = 10;
	static const uint32_t TRIANGLE_SCREEN_SIZE = 50;

	typedef struct _IcoTriangle
	{
		// A triangle consists of 3 vertices, p, p0 and p1
		Vector3f	p;		// One vertex of a triangle
		Vector3f	v0;		// Side vector v0 = p0 - p
		Vector3f	v1;		// Side vector v1 = p1 - p
	}IcoTriangle;

public:
	static std::shared_ptr<PlanetGenerator> Create(const std::shared_ptr<PhysicalCamera>& pCamera);

protected:
	bool Init(const std::shared_ptr<PlanetGenerator>& pSelf, const std::shared_ptr<PhysicalCamera>& pCamera);

protected:
	void SubDivide(uint32_t currentLevel, const Vector3f& a, const Vector3f& b, const Vector3f& c, IcoTriangle*& pOutputTriangles);

public:
	void Start() override;
	void OnPreRender() override;

public:
	void ToggleCameraInfoUpdate(bool flag) { m_toggleCameraInfoUpdate = flag; }

private:
	Vector3f		m_icosahedronVertices[20];
	uint32_t		m_icosahedronIndices[20 * 3];

	std::vector<float>				m_distanceLUT;
	std::shared_ptr<MeshRenderer>	m_pMeshRenderer;
	std::shared_ptr<PhysicalCamera>	m_pCamera;

	// Utility variables, to avoid frequent construction and destruction every frame
	Matrix4f		m_utilityTransfrom;
	Vector3f		m_utilityVector0;
	Vector3f		m_utilityVector1;

	// Camera infor in planet local space
	PyramidFrustumf	m_cameraFrustumLocal;
	Vector3f		m_cameraPosLocal;

	// Whether to update camera info in planet local space
	// This is used mostly for debugging
	// You can investigate culling result around by setting it to false
	bool			m_toggleCameraInfoUpdate = true;

};