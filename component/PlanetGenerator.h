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

	static const uint32_t MAX_LEVEL = 8;
	static const uint32_t TRIANGLE_SCREEN_SIZE = 200;

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

protected:
	bool Init(const std::shared_ptr<PlanetGenerator>& pSelf, const std::shared_ptr<PhysicalCamera>& pCamera);

protected:
	CullState FrustumCull(const Vector3f& a, const Vector3f& b, const Vector3f& c);
	void SubDivide(uint32_t currentLevel, CullState state, const Vector3f& a, const Vector3f& b, const Vector3f& c, IcoTriangle*& pOutputTriangles);

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