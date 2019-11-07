#include "PlanetGenerator.h"
#include "MeshRenderer.h"
#include "../Base/BaseObject.h"
#include "../class/PlanetGeoDataManager.h"


DEFINITE_CLASS_RTTI(PlanetGenerator, BaseComponent);

std::shared_ptr<PlanetGenerator> PlanetGenerator::Create()
{
	std::shared_ptr<PlanetGenerator> pPlanetGenerator = std::make_shared<PlanetGenerator>();
	if (pPlanetGenerator.get() && pPlanetGenerator->Init(pPlanetGenerator))
		return pPlanetGenerator;
	return nullptr;
}

bool PlanetGenerator::Init(const std::shared_ptr<PlanetGenerator>& pSelf)
{
	if (!BaseComponent::Init(pSelf))
		return false;

	float ratio = (1.0f + sqrt(5.0f)) / 2.0f;
	float scale = 1.0f / Vector2f(ratio, 1.0f).Length();
	ratio *= scale;

	Vector3f vertices[] =
	{
		{ ratio, 0, -scale },			//rf 0
		{ -ratio, 0, -scale },		//lf 1
		{ ratio, 0, scale },			//rb 2
		{ -ratio, 0, scale },			//lb 3

		{ 0, -scale, ratio },			//db 4
		{ 0, -scale, -ratio },		//df 5
		{ 0, scale, ratio },			//ub 6
		{ 0, scale, -ratio },			//uf 7

		{ -scale, ratio, 0 },			//lu 8
		{ -scale, -ratio, 0 },		//ld 9
		{ scale, ratio, 0 },			//ru 10
		{ scale, -ratio, 0 }			//rd 11
	};

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

	return true;
}

void PlanetGenerator::Start()
{
	m_pMeshRenderer = GetComponent<MeshRenderer>();
	//ASSERTION(m_pMeshRenderer != nullptr);
}

void PlanetGenerator::SubDivide(uint32_t currentLevel, uint32_t targetLevel, const Vector3f& a, const Vector3f& b, const Vector3f& c, IcoTriangle*& pOutputTriangles)
{
	if (currentLevel == targetLevel)
	{
		pOutputTriangles->p = a;
		pOutputTriangles->v0 = b - a;
		pOutputTriangles->v1 = c - a;
		pOutputTriangles++;
		return;
	}

	Vector3f A = b + (c - b) * 0.5f;
	Vector3f B = a + (c - a) * 0.5f;
	Vector3f C = a + (b - a) * 0.5f;

	SubDivide(currentLevel + 1, targetLevel, a, C, B, pOutputTriangles);
	SubDivide(currentLevel + 1, targetLevel, C, b, A, pOutputTriangles);
	SubDivide(currentLevel + 1, targetLevel, B, A, c, pOutputTriangles);
	SubDivide(currentLevel + 1, targetLevel, A, B, C, pOutputTriangles);
}

void PlanetGenerator::Update()
{
	uint32_t offsetInBytes;

	IcoTriangle* pTriangles = (IcoTriangle*)PlanetGeoDataManager::GetInstance()->AcquireDataPtr(offsetInBytes);
	uint8_t* startPtr = (uint8_t*)pTriangles;

	for (uint32_t i = 0; i < 20; i++)
	{
		SubDivide(0, 2, m_icosahedronVertices[m_icosahedronIndices[i * 3]], m_icosahedronVertices[m_icosahedronIndices[i * 3 + 1]], m_icosahedronVertices[m_icosahedronIndices[i * 3 + 2]], pTriangles);
	}

	uint32_t updatedSize = (uint8_t*)pTriangles - startPtr;

	PlanetGeoDataManager::GetInstance()->FinishDataUpdate(updatedSize);
	
	if (m_pMeshRenderer != nullptr)
	{
		m_pMeshRenderer->SetStartInstance(offsetInBytes / sizeof(IcoTriangle));
		m_pMeshRenderer->SetInstanceCount(updatedSize / sizeof(IcoTriangle));
	}
}