#include "PlanetGenerator.h"
#include "MeshRenderer.h"
#include "../Base/BaseObject.h"
#include "../class/PlanetGeoDataManager.h"
#include "PhysicalCamera.h"

#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/FrameManager.h"
#include <fstream>

DEFINITE_CLASS_RTTI(PlanetGenerator, BaseComponent);

std::shared_ptr<PlanetGenerator> PlanetGenerator::Create(const std::shared_ptr<PhysicalCamera>& pCamera)
{
	std::shared_ptr<PlanetGenerator> pPlanetGenerator = std::make_shared<PlanetGenerator>();
	if (pPlanetGenerator.get() && pPlanetGenerator->Init(pPlanetGenerator, pCamera))
		return pPlanetGenerator;
	return nullptr;
}

bool PlanetGenerator::Init(const std::shared_ptr<PlanetGenerator>& pSelf, const std::shared_ptr<PhysicalCamera>& pCamera)
{
	if (!BaseComponent::Init(pSelf))
		return false;

	m_pCamera = pCamera;

	float ratio = (1.0f + sqrt(5.0f)) / 2.0f;
	float scale = 1.0f / Vector2f(ratio, 1.0f).Length();
	ratio *= scale;

	Vector3f vertices[] =
	{
		{ ratio, 0, -scale },			//rf 0
		{ -ratio, 0, -scale },			//lf 1
		{ ratio, 0, scale },			//rb 2
		{ -ratio, 0, scale },			//lb 3

		{ 0, -scale, ratio },			//db 4
		{ 0, -scale, -ratio },			//df 5
		{ 0, scale, ratio },			//ub 6
		{ 0, scale, -ratio },			//uf 7

		{ -scale, ratio, 0 },			//lu 8
		{ -scale, -ratio, 0 },			//ld 9
		{ scale, ratio, 0 },			//ru 10
		{ scale, -ratio, 0 }			//rd 11
	};

	for (uint32_t i = 0; i < 12; i++)
	{
		vertices[i].Normalize();
	}

	memcpy_s(&m_icosahedronVertices, sizeof(m_icosahedronVertices), &vertices, sizeof(vertices));

	uint32_t indices[20 * 3] =
	{
		1, 3, 8,
		3, 1, 9,
		0, 10, 2,
		2, 11, 0,

		5, 7, 0,
		7, 5, 1,
		4, 2, 6,
		6, 3, 4,

		9, 11, 4,
		11, 9, 5,
		8, 6, 10,
		10, 7, 8,

		1, 8, 7,
		5, 9, 1,
		0, 7, 10,
		5, 0, 11,

		3, 6, 8,
		4, 3, 9,
		2, 10, 6,
		4, 11, 2
	};

	memcpy_s(&m_icosahedronIndices, sizeof(m_icosahedronIndices), &indices, sizeof(indices));

	float size = (m_icosahedronVertices[m_icosahedronIndices[0]] - m_icosahedronVertices[m_icosahedronIndices[1]]).Length();
	float frac = tanf(3.14159265f * 0.33333f * TRIANGLE_SCREEN_SIZE / 1440.0f);
	for (uint32_t i = 0; i < MAX_LEVEL + 1; i++)
	{
		m_distanceLUT.push_back(size / frac);
		size *= 0.5f;
	}

	return true;
}

void PlanetGenerator::Start()
{
	m_pMeshRenderer = GetComponent<MeshRenderer>();
	//ASSERTION(m_pMeshRenderer != nullptr);
}

void PlanetGenerator::SubDivide(uint32_t currentLevel, const Vector3f& cameraPos, const Vector3f& cameraDir, const Vector3f& a, const Vector3f& b, const Vector3f& c, IcoTriangle*& pOutputTriangles)
{
	Vector3f triangleNormal = a;
	triangleNormal += b;
	triangleNormal += c;
	triangleNormal.Normalize();
	
	if (triangleNormal * cameraDir > 0.5)
		return;

	float distA = (cameraPos - a).Length();
	float distB = (cameraPos - b).Length();
	float distC = (cameraPos - c).Length();

	float minDist = std::fminf(std::fminf(distA, distB), distC);

	if (m_distanceLUT[currentLevel] <= minDist || currentLevel == MAX_LEVEL)
	{
		pOutputTriangles->p = a;
		pOutputTriangles->v0 = b;
		pOutputTriangles->v1 = c;
		pOutputTriangles->v0 -= a;
		pOutputTriangles->v1 -= a;
		pOutputTriangles++;
		return;
	}

	Vector3f A = c;
	A -= b;
	A *= 0.5f;
	A += b;
	A.Normalize();

	Vector3f B = c;
	B -= a;
	B *= 0.5f;
	B += a;
	B.Normalize();

	Vector3f C = b;
	C -= a;
	C *= 0.5f;
	C += a;
	C.Normalize();

	SubDivide(currentLevel + 1, cameraPos, cameraDir, a, C, B, pOutputTriangles);
	SubDivide(currentLevel + 1, cameraPos, cameraDir, C, b, A, pOutputTriangles);
	SubDivide(currentLevel + 1, cameraPos, cameraDir, B, A, c, pOutputTriangles);
	SubDivide(currentLevel + 1, cameraPos, cameraDir, A, B, C, pOutputTriangles);
}

void PlanetGenerator::OnPreRender()
{
	Vector3f localCameraPosition = GetBaseObject()->GetWorldTransform().Inverse().TransformAsPoint(m_pCamera->GetBaseObject()->GetWorldPosition());
	Vector3f localCameraDirection = GetBaseObject()->GetWorldTransform().Inverse().TransformAsVector(m_pCamera->GetCameraDir());

	uint32_t offsetInBytes;

	IcoTriangle* pTriangles = (IcoTriangle*)PlanetGeoDataManager::GetInstance()->AcquireDataPtr(offsetInBytes);
	uint8_t* startPtr = (uint8_t*)pTriangles;

	for (uint32_t i = 0; i < 20; i++)
	{
		SubDivide(0, localCameraPosition, localCameraDirection, m_icosahedronVertices[m_icosahedronIndices[i * 3]], m_icosahedronVertices[m_icosahedronIndices[i * 3 + 1]], m_icosahedronVertices[m_icosahedronIndices[i * 3 + 2]], pTriangles);
	}
	uint32_t updatedSize = (uint8_t*)pTriangles - startPtr;

	PlanetGeoDataManager::GetInstance()->FinishDataUpdate(updatedSize);
	
	static uint32_t asdf[3096];
	static uint32_t qwer[3096];
	static uint32_t c = 0;
	if (m_pMeshRenderer != nullptr)
	{
		m_pMeshRenderer->SetStartInstance(offsetInBytes / sizeof(IcoTriangle));
		m_pMeshRenderer->SetInstanceCount(updatedSize / sizeof(IcoTriangle));
	}
}