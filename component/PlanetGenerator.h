#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../class/PerFrameData.h"

class MeshRenderer;

class PlanetGenerator : public BaseComponent
{
	DECLARE_CLASS_RTTI(PlanetGenerator);

	typedef struct _IcoTriangle
	{
		// A triangle consists of 3 vertices, p, p0 and p1
		Vector3f	p;		// One vertex of a triangle
		Vector3f	v0;		// Side vector v0 = p0 - p
		Vector3f	v1;		// Side vector v1 = p1 - p
	}IcoTriangle;

public:
	static std::shared_ptr<PlanetGenerator> Create();

protected:
	bool Init(const std::shared_ptr<PlanetGenerator>& pSelf);

protected:
	static void SubDivide(uint32_t currentLevel, uint32_t targetLevel, const Vector3f& a, const Vector3f& b, const Vector3f& c, IcoTriangle*& pOutputTriangles);

public:
	void Start() override;
	void Update() override;

private:
	Vector3f	m_icosahedronVertices[20];
	uint32_t	m_icosahedronIndices[20 * 3];

	std::shared_ptr<MeshRenderer>	m_pMeshRenderer;
};