#include "PlanetGenerator.h"
#include "MeshRenderer.h"
#include "../Base/BaseObject.h"
#include "../class/PlanetGeoDataManager.h"
#include "../Maths/Plane.h"
#include "../Maths/MathUtil.h"
#include "../scene/SceneGenerator.h"
#include "../class/UniformData.h"
#include "PhysicalCamera.h"

DEFINITE_CLASS_RTTI(PlanetGenerator, BaseComponent);

std::shared_ptr<PlanetGenerator> PlanetGenerator::Create(const std::shared_ptr<PhysicalCamera>& pCamera, float planetRadius)
{
	std::shared_ptr<PlanetGenerator> pPlanetGenerator = std::make_shared<PlanetGenerator>();
	if (pPlanetGenerator.get() && pPlanetGenerator->Init(pPlanetGenerator, pCamera, planetRadius))
		return pPlanetGenerator;
	return nullptr;
}

bool PlanetGenerator::Init(const std::shared_ptr<PlanetGenerator>& pSelf, const std::shared_ptr<PhysicalCamera>& pCamera, float planetRadius)
{
	if (!BaseComponent::Init(pSelf))
		return false;

	m_pCamera = pCamera;
	m_planetRadius = planetRadius;

	Vector3d a = UniformData::GetInstance()->GetGlobalUniforms()->CubeVertices[UniformData::GetInstance()->GetGlobalUniforms()->CubeIndices[1]];
	Vector3d b = UniformData::GetInstance()->GetGlobalUniforms()->CubeVertices[UniformData::GetInstance()->GetGlobalUniforms()->CubeIndices[2]];
	Vector3d center = (a + b) / 2.0;
	center.Normalize();

	double cosin_a_center = a * center;

	// cosin_a_center = r / h, r = 1(local length)
	double height_level_0 = 1 / cosin_a_center;
	
	m_heightLUT.push_back(height_level_0);
	for (uint32_t i = 1; i < PLANET_LOD_MAX_LEVEL + 1; i++)
	{
		// Next level vertices
		Vector3d A = center.Normal();
		Vector3d B = b;

		center = (A + B) * 0.5;
		center.Normalize();

		double cosin_A_center = A * center;
		double height = 1 / cosin_A_center;
		m_heightLUT.push_back(height);

		a = A;
		b = B;
	}

	return true;
}

void PlanetGenerator::Start()
{
	m_pMeshRenderer = GetComponent<MeshRenderer>();

	m_maxLODLevel = (uint32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetMaxPlanetLODLevel();

	// Make a copy here
	for (uint32_t i = 0; i < m_maxLODLevel; i++)
		m_distanceLUT.push_back(UniformData::GetInstance()->GetGlobalUniforms()->GetLODDistance(i));

	m_pVertices = UniformData::GetInstance()->GetGlobalUniforms()->CubeVertices;
	m_pIndices = UniformData::GetInstance()->GetGlobalUniforms()->CubeIndices;

	//ASSERTION(m_pMeshRenderer != nullptr);
}

PlanetGenerator::CullState PlanetGenerator::FrustumCull(const Vector3d& a, const Vector3d& b, const Vector3d& c, double height)
{
	CullState state = CullState::DIVIDE;
	for (uint32_t i = 0; i < m_cameraFrustumLocal.FrustumFace_COUNT; i++)
	{
		uint32_t outsideCount = 0;
		outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(a) > 0 ? 0 : 1;
		outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(b) > 0 ? 0 : 1;
		outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(c) > 0 ? 0 : 1;

		if (outsideCount == 3)
		{
			outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(a * height) > 0 ? 0 : 1;
			outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(b * height) > 0 ? 0 : 1;
			outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(c * height) > 0 ? 0 : 1;

			if (outsideCount == 6)
				return CullState::CULL;
			else
				state = CullState::CULL_DIVIDE;
		}
		else if (outsideCount > 0)
			state = CullState::CULL_DIVIDE;
	}

	return state;
}

PlanetGenerator::CullState PlanetGenerator::FrustumCull(const Vector3d& p0, const Vector3d& p1, const Vector3d& p2, const Vector3d& p3, double height)
{
	CullState state = CullState::DIVIDE;
	for (uint32_t i = 0; i < m_cameraFrustumLocal.FrustumFace_COUNT; i++)
	{
		uint32_t outsideCount = 0;
		outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(p0) > 0 ? 0 : 1;
		outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(p1) > 0 ? 0 : 1;
		outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(p2) > 0 ? 0 : 1;
		outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(p3) > 0 ? 0 : 1;

		if (outsideCount == 4)
		{
			outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(p0 * height) > 0 ? 0 : 1;
			outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(p1 * height) > 0 ? 0 : 1;
			outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(p2 * height) > 0 ? 0 : 1;
			outsideCount += m_cameraFrustumLocal.planes[i].PlaneTest(p3 * height) > 0 ? 0 : 1;

			if (outsideCount == 8)
				return CullState::CULL;
			else
				state = CullState::CULL_DIVIDE;
		}
		else if (outsideCount > 0)
			state = CullState::CULL_DIVIDE;
	}

	return state;
}

bool PlanetGenerator::BackFaceCull(const Vector3d& a, const Vector3d& b, const Vector3d& c)
{
	// Utility vector No.3 represents triangle normal
	// Utility vector No.4 represents vector from camera to one triangle vertex
	m_utilityVector3 = c;
	m_utilityVector4 = a;
	m_utilityVector3 -= b;
	m_utilityVector4 -= b;
	m_utilityVector3 = m_utilityVector3 ^ m_utilityVector4;

	m_utilityVector4 = a;
	m_utilityVector4 -= m_lockedPlanetSpaceCameraPosition;

	// If camera is located at the negative side of this triangle(dot product greater than 0)
	// Then more check will perform
	// Otherwise(camera at positive side of this triangle), proceeding to subdivide step
	// NOTE: No need to normalize, as what we want is the dot product sign only
	if (m_utilityVector3 * m_utilityVector4 > 0.0)
	{
		// Utility vector 0, 1 and 2 are vectors from camera to them respectively
		m_utilityVector0 = a;
		m_utilityVector1 = b;
		m_utilityVector2 = c;

		m_utilityVector0 -= m_lockedPlanetSpaceCameraPosition;
		m_utilityVector1 -= m_lockedPlanetSpaceCameraPosition;
		m_utilityVector2 -= m_lockedPlanetSpaceCameraPosition;

		// This checks if camera could observe the sphere surface within current triangle 
		// NOTE: No need to normalize, as what we want is the dot product sign only
		if (a * m_utilityVector0 > 0.0
			&& b * m_utilityVector1 > 0.0
			&& c * m_utilityVector2 > 0.0)
			return true;
	}

	return false;
}

// Assume a quad is arranged like this
// c--------d
// | \      |
// |   \    |
// |     \  |
// a--------b
// trianlge 1: abc, triangle 2: cbd
bool PlanetGenerator::BackFaceCull(const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d)
{
	// Triangle 1 start
	// Utility vector No.3 represents triangle normal
	// Utility vector No.4 represents vector from camera to one triangle vertex
	m_utilityVector3 = c;
	m_utilityVector4 = a;
	m_utilityVector3 -= b;
	m_utilityVector4 -= b;
	m_utilityVector3 = m_utilityVector3 ^ m_utilityVector4;

	m_utilityVector4 = a;
	m_utilityVector4 -= m_lockedPlanetSpaceCameraPosition;

	// If camera is located at the negative side of this triangle(dot product greater than 0)
	// Then more check will perform
	// Otherwise(camera at positive side of this triangle), proceeding to subdivide step
	// NOTE: No need to normalize, as what we want is the dot product sign only
	if (m_utilityVector3 * m_utilityVector4 < 0.0)
		return false;

	// Utility vector 0, 1 and 2 are vectors from camera to them respectively
	m_utilityVector0 = a;
	m_utilityVector1 = b;
	m_utilityVector2 = c;

	m_utilityVector0 -= m_lockedPlanetSpaceCameraPosition;
	m_utilityVector1 -= m_lockedPlanetSpaceCameraPosition;
	m_utilityVector2 -= m_lockedPlanetSpaceCameraPosition;

	// This checks if camera could observe the sphere surface within current triangle 
	// NOTE: No need to normalize, as what we want is the dot product sign only
	if (a * m_utilityVector0 < 0.0
		|| b * m_utilityVector1 < 0.0
		|| c * m_utilityVector2 < 0.0)
		return false;

	// Now we're done for checking trianlge 1, now start triangle 2
	m_utilityVector3 = d;
	m_utilityVector4 = c;
	m_utilityVector3 -= b;
	m_utilityVector4 -= b;
	m_utilityVector3 = m_utilityVector3 ^ m_utilityVector4;

	m_utilityVector4 = d;
	m_utilityVector4 -= m_lockedPlanetSpaceCameraPosition;

	if (m_utilityVector3 * m_utilityVector4 < 0.0)
		return false;

	// Only deal with 4th vertex
	m_utilityVector0 = d;

	m_utilityVector0 -= m_lockedPlanetSpaceCameraPosition;

	// This checks if camera could observe the sphere surface within current triangle 
	// NOTE: No need to normalize, as what we want is the dot product sign only
	if (d * m_utilityVector0 < 0.0)
		return false;

	// Finally we're sure that nothing can be seen from this angle, we return true as a permission to cull this quad
	return true;
}

void PlanetGenerator::SubDivideTriangle(uint32_t currentLevel, CullState state, const Vector3d& a, const Vector3d& b, const Vector3d& c, bool reversed, Triangle*& pOutputTriangles)
{
	Vector3d newA = a;
	Vector3d newB = b;
	Vector3d newC = c;

	newA *= m_planetRadius;
	newB *= m_planetRadius;
	newC *= m_planetRadius;

	// Only perform frustum cull if state is CULL_DIVIDE, as it intersects the volumn
	if (state == CullState::CULL_DIVIDE)
	{
		// Frustum cull
		state = FrustumCull(newA, newB, newC, m_heightLUT[currentLevel]);

		// Early quit if triangle is totally outside of the volumn
		if (state == CullState::CULL)
			return;

		// Whatever has left should be either CULL_DIVIDE or DIVIDE
		// This state will be passed on to the next sub divide level
	}

	if (BackFaceCull(newA, newB, newC))
		return;

	Vector3d camera_relative_a = newA - m_lockedPlanetSpaceCameraPosition;
	Vector3d camera_relative_b = newB - m_lockedPlanetSpaceCameraPosition;
	Vector3d camera_relative_c = newC - m_lockedPlanetSpaceCameraPosition;

	double distA = camera_relative_a.Length();
	double distB = camera_relative_b.Length();
	double distC = camera_relative_c.Length();

	double minDist = std::fmin(std::fmin(distA, distB), distC);

	if (m_distanceLUT[currentLevel] * m_planetRadius <= minDist || currentLevel == m_maxLODLevel)
	{
		if (!m_toggleCameraInfoUpdate)
		{
			camera_relative_a = newA - m_planetSpaceCameraPosition;
			camera_relative_b = newB - m_planetSpaceCameraPosition;
			camera_relative_c = newC - m_planetSpaceCameraPosition;
		}

		pOutputTriangles->p = camera_relative_a.SinglePrecision();
		pOutputTriangles->edge0 = camera_relative_b.SinglePrecision();
		pOutputTriangles->edge1 = camera_relative_c.SinglePrecision();

		pOutputTriangles->edge0 -= pOutputTriangles->p;
		pOutputTriangles->edge1 -= pOutputTriangles->p;

		// Level + 1 to avoid zero
		pOutputTriangles->level = reversed ? -1.0f : 1.0f * (currentLevel + 1);

		pOutputTriangles++;

		return;
	}

	Vector3d A = c;
	Vector3d B = c;
	Vector3d C = b;

	A += b;
	A *= 0.5f;

	B += a;
	B *= 0.5f;

	C += a;
	C *= 0.5f;

	A.Normalize();
	B.Normalize();
	C.Normalize();

	SubDivideTriangle(currentLevel + 1, state, a, C, B, reversed, pOutputTriangles);
	SubDivideTriangle(currentLevel + 1, state, C, b, A, reversed, pOutputTriangles);
	SubDivideTriangle(currentLevel + 1, state, B, A, c, reversed, pOutputTriangles);
	SubDivideTriangle(currentLevel + 1, state, A, B, C, reversed, pOutputTriangles);
}

void PlanetGenerator::SubDivideQuad(uint32_t currentLevel, CullState state, const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d, Triangle*& pOutputTriangles)
{
	Vector3d realSizeA = a;
	Vector3d realSizeB = b;
	Vector3d realSizeC = c;
	Vector3d realSizeD = d;

	realSizeA.Normalize();
	realSizeB.Normalize();
	realSizeC.Normalize();
	realSizeD.Normalize();

	realSizeA *= m_planetRadius;
	realSizeB *= m_planetRadius;
	realSizeC *= m_planetRadius;
	realSizeD *= m_planetRadius;

	// Only perform frustum cull if state is CULL_DIVIDE, as it intersects the volumn
	if (state == CullState::CULL_DIVIDE)
	{
		// Frustum cull
		state = FrustumCull(realSizeA, realSizeB, realSizeC, realSizeD, m_heightLUT[currentLevel]);

		// Early quit if triangle is totally outside of the volumn
		if (state == CullState::CULL)
			return;

		// Whatever has left should be either CULL_DIVIDE or DIVIDE
		// This state will be passed on to the next sub divide level
	}

	if (BackFaceCull(realSizeA, realSizeB, realSizeC, realSizeD))
		return;

	Vector3d camera_relative_a = realSizeA - m_lockedPlanetSpaceCameraPosition;
	Vector3d camera_relative_b = realSizeB - m_lockedPlanetSpaceCameraPosition;
	Vector3d camera_relative_c = realSizeC - m_lockedPlanetSpaceCameraPosition;
	Vector3d camera_relative_d = realSizeD - m_lockedPlanetSpaceCameraPosition;

	double distA = camera_relative_a.Length();
	double distB = camera_relative_b.Length();
	double distC = camera_relative_c.Length();
	double distD = camera_relative_d.Length();

	double minDist = std::fmin(std::fmin(std::fmin(distA, distB), distC), distD);

	if (m_distanceLUT[currentLevel] * m_planetRadius <= minDist || currentLevel == m_maxLODLevel)
	{
		if (!m_toggleCameraInfoUpdate)
		{
			camera_relative_a = realSizeA - m_planetSpaceCameraPosition;
			camera_relative_b = realSizeB - m_planetSpaceCameraPosition;
			camera_relative_c = realSizeC - m_planetSpaceCameraPosition;
			camera_relative_d = realSizeD - m_planetSpaceCameraPosition;
		}

		// Triangle abc
		pOutputTriangles->p = camera_relative_a.SinglePrecision();
		pOutputTriangles->edge0 = camera_relative_b.SinglePrecision();
		pOutputTriangles->edge1 = camera_relative_c.SinglePrecision();

		pOutputTriangles->edge0 -= pOutputTriangles->p;
		pOutputTriangles->edge1 -= pOutputTriangles->p;

		// Level + 1 to avoid zero
		pOutputTriangles->level = (float)currentLevel + 1.0f;

		pOutputTriangles++;

		// Triangle cbd
		pOutputTriangles->p = camera_relative_d.SinglePrecision();
		pOutputTriangles->edge0 = camera_relative_c.SinglePrecision();
		pOutputTriangles->edge1 = camera_relative_b.SinglePrecision();

		pOutputTriangles->edge0 -= pOutputTriangles->p;
		pOutputTriangles->edge1 -= pOutputTriangles->p;

		// Minus gives a sign whether to reverse morphing in vertex shader
		pOutputTriangles->level = pOutputTriangles->level * -1.0f;

		pOutputTriangles++;

		return;
	}

	Vector3d ab = a;
	Vector3d ac = a;
	Vector3d bd = b;
	Vector3d cd = c;

	ab += b;
	ab *= 0.5f;

	ac += c;
	ac *= 0.5f;

	bd += d;
	bd *= 0.5f;

	cd += d;
	cd *= 0.5f;

	Vector3d center = ab;
	center += cd;
	center *= 0.5f;

	SubDivideQuad(currentLevel + 1, state, a, ab, ac, center, pOutputTriangles);
	SubDivideQuad(currentLevel + 1, state, ab, b, center, bd, pOutputTriangles);
	SubDivideQuad(currentLevel + 1, state, ac, center, c, cd, pOutputTriangles);
	SubDivideQuad(currentLevel + 1, state, center, bd, cd, d, pOutputTriangles);
}

void PlanetGenerator::OnPreRender()
{
	// Transform from world space to planet local space
	m_utilityTransfrom = GetBaseObject()->GetCachedWorldTransform();
	m_utilityTransfrom.Inverse();

	m_planetSpaceCameraPosition = m_utilityTransfrom.TransformAsPoint(m_pCamera->GetBaseObject()->GetCachedWorldPosition());

	// Transfrom from camera local space to world space, and then to planet local space
	m_utilityTransfrom *= m_pCamera->GetBaseObject()->GetCachedWorldTransform();	// from camera local 2 world

	if (m_toggleCameraInfoUpdate)
	{
		m_lockedPlanetSpaceCameraPosition = m_planetSpaceCameraPosition;

		m_cameraFrustumLocal = m_pCamera->GetCameraFrustum();
		m_cameraFrustumLocal.Transform(m_utilityTransfrom);
	}

	uint32_t offsetInBytes;

	Triangle* pTriangles = (Triangle*)PlanetGeoDataManager::GetInstance()->AcquireDataPtr(offsetInBytes);
	uint8_t* startPtr = (uint8_t*)pTriangles;

	for (uint32_t i = 0; i < 6; i++)
	{
		SubDivideQuad(0, CullState::CULL_DIVIDE,
			m_pVertices[m_pIndices[i * 6 + 0]],	// a
			m_pVertices[m_pIndices[i * 6 + 1]],	// b
			m_pVertices[m_pIndices[i * 6 + 2]],	// c
			m_pVertices[m_pIndices[i * 6 + 5]],	// d
			pTriangles);
	}
	uint32_t updatedSize = (uint32_t)((uint8_t*)pTriangles - startPtr);

	PlanetGeoDataManager::GetInstance()->FinishDataUpdate(updatedSize);
	
	if (m_pMeshRenderer != nullptr)
	{
		m_pMeshRenderer->SetStartInstance(offsetInBytes / sizeof(Triangle));
		m_pMeshRenderer->SetInstanceCount(updatedSize / sizeof(Triangle));
	}
}