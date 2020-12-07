#include "PlanetGenerator.h"
#include "MeshRenderer.h"
#include "../Base/BaseObject.h"
#include "../class/PlanetGeoDataManager.h"
#include "../Maths/Plane.h"
#include "../Maths/MathUtil.h"
#include "../scene/SceneGenerator.h"
#include "../class/UniformData.h"
#include "PhysicalCamera.h"
#include "../class/PerPlanetUniforms.h"

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

	m_chunkIndex = UniformData::GetInstance()->GetPerPerPlanetUniforms()->AllocatePlanetChunk();
	UniformData::GetInstance()->GetPerPerPlanetUniforms()->SetPlanetRadius(m_chunkIndex, m_planetRadius);

	m_squarePlanetRadius = m_planetRadius * m_planetRadius;

	m_tileMask = 0;
	m_cubeFaceNormals[CubeFace::RIGHT]	= {  1,  0,  0 };
	m_cubeFaceNormals[CubeFace::LEFT]	= { -1,  0,  0 };
	m_cubeFaceNormals[CubeFace::TOP]	= {  0,  1,  0 };
	m_cubeFaceNormals[CubeFace::BOTTOM] = {  0, -1,  0 };
	m_cubeFaceNormals[CubeFace::FRONT]	= {  0,  0,  1 };
	m_cubeFaceNormals[CubeFace::BACK]	= {  0,  0, -1 };

	return true;
}

void PlanetGenerator::Start()
{
	m_pMeshRenderer = GetComponent<MeshRenderer>();

	m_maxLODLevel = (uint32_t)UniformData::GetInstance()->GetPerPerPlanetUniforms()->GetPlanetMAXLODLevel(m_chunkIndex);

	// Make a copy here
	for (uint32_t i = 0; i < m_maxLODLevel; i++)
		m_distanceLUT.push_back(UniformData::GetInstance()->GetPerPerPlanetUniforms()->GetLODDistance(m_chunkIndex, i));

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

double PlanetGenerator::ComputeDistanceToTile
(
	const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d,
	const Vector3d& realSizeA, const Vector3d& realSizeB, const Vector3d& realSizeC, const Vector3d& realSizeD, 
	const Vector3d& faceNormal, const double currentTileLength, CubeFace cubeFace
)
{
	// Step 2
	double cosineTheta = faceNormal * m_lockedNormalizedPlanetSpaceCameraPosition;

	// Step 3
	static double halfCubeEdgeLength = std::sqrt(3.0) / 3.0 * m_pVertices[0].Length();
	Vector3d camVecInsideCube = m_lockedNormalizedPlanetSpaceCameraPosition;
	camVecInsideCube *= (halfCubeEdgeLength / cosineTheta);

	// Step 4
	uint32_t axisX, axisY;
	if (cubeFace == CubeFace::BOTTOM || cubeFace == CubeFace::TOP)
	{
		axisX = 0; axisY = 2;
	}
	else if (cubeFace == CubeFace::FRONT || cubeFace == CubeFace::BACK)
	{
		axisX = 0, axisY = 1;
	}
	else
	{
		axisX = 1, axisY = 2;
	}

	double lowerX, higherX, lowerY, higherY;
	if (a[axisX] <= d[axisX])
	{
		lowerX = a[axisX];
		higherX = d[axisX];
	}
	else
	{
		lowerX = d[axisX];
		higherX = a[axisX];
	}

	if (a[axisY] <= d[axisY])
	{
		lowerY = a[axisY];
		higherY = d[axisY];
	}
	else
	{
		lowerY = d[axisY];
		higherY = a[axisY];
	}

	bool lowerThanXRange = lowerX > camVecInsideCube[axisX];
	bool higherThanXRange = higherX < camVecInsideCube[axisX];
	bool lowerThanYRange = lowerY > camVecInsideCube[axisY];
	bool higherThanYRange = higherY < camVecInsideCube[axisY];
	bool withinXRange = !lowerThanXRange && !higherThanXRange;
	bool withinYRange = !lowerThanYRange && !higherThanYRange;

	auto GetDistance = [&](const Vector3d& targetPointVector)
	{
		double cosineTheta = targetPointVector * m_lockedNormalizedPlanetSpaceCameraPosition;

		// Triangle: c ^ 2 = a ^ 2 + b ^ 2 - 2 * a * b * cosineTheta
		// For earth: x ^ 2 = r ^ 2 + (r + h) ^ 2 + 2 * r * (r + h) * consineTheta
		return std::sqrt(
			m_squareLockedPlanetSpaceCameraHeight
			+ m_squarePlanetRadius
			- 2.0 * m_planetRadius * m_lockedPlanetSpaceCameraHeight * cosineTheta
			);
	};

	if (withinXRange && withinYRange)
	{
		return m_lockedPlanetSpaceCameraHeight - m_planetRadius;
	}
	else if (withinXRange || withinYRange)
	{
		double* pLower, *pHigher, *pVal, *pTheOtherAxis;
		if (withinXRange)
		{
			pLower = &lowerX;
			pHigher = &higherX;
			pVal = &camVecInsideCube[axisX];
			pTheOtherAxis = lowerThanYRange ? &lowerY : &higherY;
		}
		else
		{
			pLower = &lowerY;
			pHigher = &higherY;
			pVal = &camVecInsideCube[axisY];
			pTheOtherAxis = lowerThanXRange ? &lowerX : &higherX;
		}

		double length = *pVal - *pLower;
		double ratio = length / currentTileLength;

		// Interpolation
		double interpolated = (1.0 - ratio) * *pLower + ratio * *pHigher;
		Vector3d nearestPoint(a);
		if (withinXRange)
		{
			nearestPoint[axisX] = interpolated;
			nearestPoint[axisY] = *pTheOtherAxis;
		}
		else
		{
			nearestPoint[axisX] = *pTheOtherAxis;
			nearestPoint[axisY] = interpolated;
		}

		nearestPoint.Normalize();

		double d = GetDistance(nearestPoint);
		return d;
	}
	else
	{
		double* pBoundaryX[2] = { &lowerX, &higherX };
		double* pBoundaryY[2] = { &lowerY, &higherY };

		uint32_t indexX = lowerThanXRange ? 0 : 1;
		uint32_t indexY = lowerThanYRange ? 0 : 1;

		Vector3d nearestPoint(a);
		nearestPoint[axisX] = *pBoundaryX[indexX];
		nearestPoint[axisY] = *pBoundaryY[indexY];

		nearestPoint.Normalize();

		double d = GetDistance(nearestPoint);
		return d;
	}
}

void PlanetGenerator::SubDivideQuad
(
	uint32_t currentLevel,
	CubeFace cubeFace,
	double currentTileLength,
	CullState state,
	const Vector3d& a, const Vector3d& b, const Vector3d& c, const Vector3d& d,
	const Vector3d& faceNormal,
	Triangle*& pOutputTriangles
)
{
	Vector3d realSizeA = a;
	Vector3d realSizeB = b;
	Vector3d realSizeC = c;
	Vector3d realSizeD = d;

	realSizeA.Normalize();
	realSizeB.Normalize();
	realSizeC.Normalize();
	realSizeD.Normalize();

	bool useVertexMinDist = true;
	if (currentLevel == 0)
	{
		Vector3d center = realSizeA;
		center += realSizeB;
		center += realSizeC;
		center += realSizeD;

		center.Normalize();

		double dot = std::abs(m_lockedPlanetSpaceCameraPosition.Normal() * center);
		double dotA = realSizeA * center;
		double dotB = realSizeB * center;
		double dotC = realSizeC * center;
		double dotD = realSizeD * center;

		useVertexMinDist = (dot < dotA && dot < dotB && dot < dotC && dot < dotD);
	}

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

	//double dist = m_lockedPlanetSpaceCameraPosition.Length() - GetPlanetRadius();
	/*Vector3d normal = (camera_relative_a - camera_relative_b) ^ (camera_relative_a - camera_relative_c);
	normal.Normalize();
	double dist = std::abs(normal * camera_relative_a);
	double distA = camera_relative_a.Length();
	double distB = camera_relative_b.Length();
	double distC = camera_relative_c.Length();
	double distD = camera_relative_d.Length();

	double minDist = useVertexMinDist ? (std::fmin(std::fmin(std::fmin(distA, distB), distC), distD)) : m_lockedPlanetSpaceCameraPosition.Length() - GetPlanetRadius();*/

	double minDist = ComputeDistanceToTile(a, b, c, d, realSizeA, realSizeB, realSizeC, realSizeD, faceNormal, currentTileLength, cubeFace);

	if (currentLevel + 1 == m_maxLODLevel || m_distanceLUT[currentLevel] <= minDist)
	{
		if (!m_toggleCameraInfoUpdate)
		{
			camera_relative_a = realSizeA - m_planetSpaceCameraPosition;
			camera_relative_b = realSizeB - m_planetSpaceCameraPosition;
			camera_relative_c = realSizeC - m_planetSpaceCameraPosition;
			camera_relative_d = realSizeD - m_planetSpaceCameraPosition;
		}

		// Triangle abc
		pOutputTriangles->p = camera_relative_c.SinglePrecision();
		pOutputTriangles->edge0 = camera_relative_a.SinglePrecision();
		pOutputTriangles->edge1 = camera_relative_b.SinglePrecision();

		pOutputTriangles->edge0 -= pOutputTriangles->p;
		pOutputTriangles->edge1 -= pOutputTriangles->p;

		// Level + 1 to avoid zero
		pOutputTriangles->level = (float)currentLevel + 1.0f;

		pOutputTriangles++;

		// Triangle cbd
		pOutputTriangles->p = camera_relative_b.SinglePrecision();
		pOutputTriangles->edge0 = camera_relative_d.SinglePrecision();
		pOutputTriangles->edge1 = camera_relative_c.SinglePrecision();

		pOutputTriangles->edge0 -= pOutputTriangles->p;
		pOutputTriangles->edge1 -= pOutputTriangles->p;

		// Minus gives a sign whether to reverse morphing in vertex shader
		pOutputTriangles->level = ((float)currentLevel + 1.0f) * -1.0f;

		pOutputTriangles++;

		return;
	}

	Vector3d ab = a;
	Vector3d ac = a;
	Vector3d bd = b;
	Vector3d cd = c;

	ab += b;
	ab *= 0.5;

	ac += c;
	ac *= 0.5;

	bd += d;
	bd *= 0.5;

	cd += d;
	cd *= 0.5;

	Vector3d center = ab;
	center += cd;
	center *= 0.5;

	currentTileLength *= 0.5;

	SubDivideQuad(currentLevel + 1, cubeFace, currentTileLength, state, a, ab, ac, center, faceNormal, pOutputTriangles);
	SubDivideQuad(currentLevel + 1, cubeFace, currentTileLength, state, ab, b, center, bd, faceNormal, pOutputTriangles);
	SubDivideQuad(currentLevel + 1, cubeFace, currentTileLength, state, ac, center, c, cd, faceNormal, pOutputTriangles);
	SubDivideQuad(currentLevel + 1, cubeFace, currentTileLength, state, center, bd, cd, d, faceNormal, pOutputTriangles);
}

// Here we assume cube consists of 8 vertices with various -1 and 1
void PlanetGenerator::NewPlanetLODMethod()
{
	std::pair<double, CubeFace> cameraVecDotCubeFaceNormal[3];

	// Step1: find a rough cube face range that normalized camera position could lies in
	cameraVecDotCubeFaceNormal[0].second = m_lockedNormalizedPlanetSpaceCameraPosition.x < 0 ? CubeFace::LEFT : CubeFace::RIGHT;
	cameraVecDotCubeFaceNormal[1].second = m_lockedNormalizedPlanetSpaceCameraPosition.y < 0 ? CubeFace::BOTTOM : CubeFace::TOP;
	cameraVecDotCubeFaceNormal[2].second = m_lockedNormalizedPlanetSpaceCameraPosition.z < 0 ? CubeFace::BACK : CubeFace::FRONT;

	// Step2: Acquire maximum value of the 3 axis of normalized camera position
	cameraVecDotCubeFaceNormal[0].first = std::abs(m_lockedNormalizedPlanetSpaceCameraPosition.x);
	cameraVecDotCubeFaceNormal[1].first = std::abs(m_lockedNormalizedPlanetSpaceCameraPosition.y);
	cameraVecDotCubeFaceNormal[2].first = std::abs(m_lockedNormalizedPlanetSpaceCameraPosition.z);

	// Step3: The one with maximum axis value shall be the cube face
	uint32_t maxIndex = cameraVecDotCubeFaceNormal[0].first > cameraVecDotCubeFaceNormal[1].first ? 0 : 1;
	maxIndex = cameraVecDotCubeFaceNormal[maxIndex].first > cameraVecDotCubeFaceNormal[2].first ? maxIndex : 2;

	// Step4: Acquire cosine between normalized camera position and chosen cube face normal
	double cosineTheta = m_cubeFaceNormals[cameraVecDotCubeFaceNormal[maxIndex].second] * m_lockedNormalizedPlanetSpaceCameraPosition;

	// Step5: Acquire the cropped vector within this cube
	Vector3d camVecInsideCube = m_lockedNormalizedPlanetSpaceCameraPosition;
	camVecInsideCube *= (1 / cosineTheta);

	// Step6: Acquire axis index according to cube face
	uint32_t axisU, axisV;
	if (cameraVecDotCubeFaceNormal[maxIndex].second == CubeFace::BOTTOM || cameraVecDotCubeFaceNormal[maxIndex].second == CubeFace::TOP)
	{
		axisU = 2; axisV = 0;	// z and x
	}
	else if (cameraVecDotCubeFaceNormal[maxIndex].second == CubeFace::FRONT || cameraVecDotCubeFaceNormal[maxIndex].second == CubeFace::BACK)
	{
		axisU = 0, axisV = 1;	// x and y
	}
	else
	{
		axisU = 1, axisV = 2;	// y and z
	}

	// Step6: Acquire the normalized position(0-1) of the intersection between normalized camera position and chosen cube face
	double normU = (camVecInsideCube[axisU] + 1) / 2.0;
	double normV = (camVecInsideCube[axisV] + 1) / 2.0;

	// Reverse U if a cube face lies on the negative side of our right-hand axis
	// Do this to align UV with cube face winding order(also reversed)
	if (cameraVecDotCubeFaceNormal[maxIndex].second == CubeFace::BACK ||
		cameraVecDotCubeFaceNormal[maxIndex].second == CubeFace::LEFT ||
		cameraVecDotCubeFaceNormal[maxIndex].second == CubeFace::BOTTOM)
		normU = 1.0 - normU;

	// 1023 is the bias of exponent bits
	static uint64_t zeroExponent = 1023;
	// 52 is the count of fraction bits of double
	static uint64_t fractionBits = 52;
	// 11 is the count of exponent bits of double
	static uint64_t exponentBits = 11;
	// Extra one is the invisible one of double that is not in fraction bits for normal double
	static uint64_t extraOne = (1ull << fractionBits);
	// Fraction mask is used to extract only fraction bits of a double
	static uint64_t fractionMask = (1ull << fractionBits) - 1;
	// Exponent mask is used to extract only exponent bits of a double(You have to do right shift of "fractionBits" after)
	static uint64_t exponentMask = ((1ull << exponentBits) - 1) << fractionBits;

	// Prepare
	uint64_t *pU, *pV;
	pU = (uint64_t*)&normU;
	pV = (uint64_t*)&normV;

	// Step7: Acquire binary position of the intersection
	// 1. (*pU) & fractionMask:	Acquire only fraction bits
	// 2. (1) + extraOne:		Acquire actual fraction bits by adding an invisible bit
	// 3. (*pU) & exponentMask:	Acquire only exponent bits
	// 4. (3) >> fractionBits:	Acquire readable exponent by shifting it right of 52 bits	
	// 5. zeroExponent - (3):	We need to right shift fraction part using exponent value, to make same levels pair with same bits(floating format trait)
	// 6. (2) >> (5):			Right shift 
	uint64_t binaryU = (((*pU) & fractionMask) + extraOne) >> (zeroExponent - (((*pU) & exponentMask) >> fractionBits));
	uint64_t binaryV = (((*pV) & fractionMask) + extraOne) >> (zeroExponent - (((*pV) & exponentMask) >> fractionBits));
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
		m_lockedNormalizedPlanetSpaceCameraPosition = m_lockedPlanetSpaceCameraPosition;
		m_lockedNormalizedPlanetSpaceCameraPosition.Normalize();
		m_lockedPlanetSpaceCameraHeight = m_lockedPlanetSpaceCameraPosition.Length();
		m_squareLockedPlanetSpaceCameraHeight = m_lockedPlanetSpaceCameraHeight * m_lockedPlanetSpaceCameraHeight;

		m_cameraFrustumLocal = m_pCamera->GetCameraFrustum();
		m_cameraFrustumLocal.Transform(m_utilityTransfrom);
	}

	uint32_t offsetInBytes;

	Triangle* pTriangles = (Triangle*)PlanetGeoDataManager::GetInstance()->AcquireDataPtr(offsetInBytes);
	uint8_t* startPtr = (uint8_t*)pTriangles;

	for (uint32_t i = 0; i < CubeFace::CUBE_FACE_COUNT; i++)
	{
		Vector3d faceNormal;
		switch (i)
		{
		case 0: faceNormal = { 1, 0, 0 }; break;
		case 1: faceNormal = { -1, 0, 0 }; break;
		case 2: faceNormal = { 0, 1, 0 }; break;
		case 3: faceNormal = { 0, -1, 0 }; break;
		case 4: faceNormal = { 0, 0, 1 }; break;
		case 5: faceNormal = { 0, 0, -1 }; break;
		}
		SubDivideQuad(0, 
			(CubeFace)i,
			(m_pVertices[m_pIndices[i * 6 + 0]] - m_pVertices[m_pIndices[i * 6 + 1]]).Length(), 
			CullState::CULL_DIVIDE,
			m_pVertices[m_pIndices[i * 6 + 0]],	// a
			m_pVertices[m_pIndices[i * 6 + 1]],	// b
			m_pVertices[m_pIndices[i * 6 + 2]],	// c
			m_pVertices[m_pIndices[i * 6 + 5]],	// d
			faceNormal,
			pTriangles);
	}
	uint32_t updatedSize = (uint32_t)((uint8_t*)pTriangles - startPtr);

	PlanetGeoDataManager::GetInstance()->FinishDataUpdate(updatedSize);

	if (m_pMeshRenderer != nullptr)
	{
		m_pMeshRenderer->SetStartInstance(offsetInBytes / sizeof(Triangle));
		m_pMeshRenderer->SetInstanceCount(updatedSize / sizeof(Triangle));
		m_pMeshRenderer->SetUtilityIndex(m_chunkIndex);
	}
}