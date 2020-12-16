#include "PlanetGenerator.h"
#include "MeshRenderer.h"
#include "../Base/BaseObject.h"
#include "../class/PlanetGeoDataManager.h"
#include "../Maths/Plane.h"
#include "../Maths/MathUtil.h"
#include "../scene/SceneGenerator.h"
#include "../class/UniformData.h"
#include "../common/Util.h"
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

	m_chunkIndex = UniformData::GetInstance()->GetPerPerPlanetUniforms()->AllocatePlanetChunk();
	UniformData::GetInstance()->GetPerPerPlanetUniforms()->SetPlanetRadius(m_chunkIndex, m_planetRadius);

	m_squarePlanetRadius = m_planetRadius * m_planetRadius;

	m_tileMask = 0;
	m_cubeFaceNormals[(uint32_t)CubeFace::RIGHT]	= {  1,  0,  0 };
	m_cubeFaceNormals[(uint32_t)CubeFace::LEFT]		= { -1,  0,  0 };
	m_cubeFaceNormals[(uint32_t)CubeFace::TOP]		= {  0,  1,  0 };
	m_cubeFaceNormals[(uint32_t)CubeFace::BOTTOM]	= {  0, -1,  0 };
	m_cubeFaceNormals[(uint32_t)CubeFace::FRONT]	= {  0,  0,  1 };
	m_cubeFaceNormals[(uint32_t)CubeFace::BACK]		= {  0,  0, -1 };

	return true;
}

void PlanetGenerator::Start()
{
	m_pMeshRenderer = GetComponent<MeshRenderer>();

	// Make a copy here
	for (uint32_t i = 0; i < (uint32_t)UniformData::GetInstance()->GetPerPerPlanetUniforms()->GetPlanetMAXLODLevel(m_chunkIndex); i++)
		m_distanceLUT.push_back(UniformData::GetInstance()->GetPerPerPlanetUniforms()->GetLODDistance(m_chunkIndex, i));

	m_pVertices = UniformData::GetInstance()->GetGlobalUniforms()->CubeVertices;
	m_pIndices = UniformData::GetInstance()->GetGlobalUniforms()->CubeIndices;

	//ASSERTION(m_pMeshRenderer != nullptr);
}

// Here we assume cube consists of 8 vertices with various -1 and 1
void PlanetGenerator::NewPlanetLODMethod(Triangle*& pOutputTriangles)
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
	double cosineTheta = m_cubeFaceNormals[(uint32_t)cameraVecDotCubeFaceNormal[maxIndex].second] * m_lockedNormalizedPlanetSpaceCameraPosition;

	// Step5: Acquire the cropped vector within this cube
	Vector3d camVecInsideCube = m_lockedNormalizedPlanetSpaceCameraPosition;
	camVecInsideCube *= (1 / cosineTheta);

	CubeFace cubeFace = cameraVecDotCubeFaceNormal[maxIndex].second;

	// Step6: Acquire axis index according to cube face
	uint32_t axisU = (uint32_t)CubeFaceAxisMapping[(uint32_t)cubeFace][(uint32_t)NormCoordAxis::U];
	uint32_t axisV = (uint32_t)CubeFaceAxisMapping[(uint32_t)cubeFace][(uint32_t)NormCoordAxis::V];

	// Step6: Acquire the normalized position(0-1) of the intersection between normalized camera position and chosen cube face
	Vector2d normCoord{ (camVecInsideCube[axisU] + 1) / 2.0, (camVecInsideCube[axisV] + 1) / 2.0 };

	// Reverse U if a cube face lies on the negative side of our right-hand axis
	// Do this to align UV with cube face winding order(also reversed)
	if (cubeFace == CubeFace::BACK ||
		cubeFace == CubeFace::LEFT ||
		cubeFace == CubeFace::BOTTOM)
		normCoord.x = 1.0 - normCoord.x;

	// Prepare
	uint64_t *pU, *pV;
	pU = (uint64_t*)&normCoord.x;
	pV = (uint64_t*)&normCoord.y;

	// Step7: Acquire binary position of the intersection
	// 1. (*pU) & fractionMask:	Acquire only fraction bits
	// 2. (1) + extraOne:		Acquire actual fraction bits by adding an invisible bit
	// 3. (*pU) & exponentMask:	Acquire only exponent bits
	// 4. (3) >> fractionBits:	Acquire readable exponent by shifting it right of 52 bits	
	// 5. zeroExponent - (3):	We need to right shift fraction part using exponent value, to make same levels pair with same bits(floating format trait)
	// 6. (2) >> (5):			Right shift 
	Vector2<uint64_t> binaryCoord
	{
		AcquireBinaryCoord(normCoord.x),
		AcquireBinaryCoord(normCoord.y),
	};

	double distToGround = m_lockedPlanetSpaceCameraHeight - m_planetRadius;

	// Locate LOD level
	uint32_t level;
	for (level = 0; level < (uint32_t)m_distanceLUT.size(); level++)
		if (distToGround > m_distanceLUT[level])
			break;

	// Rebuild LOD layers
	bool rebuildLODLayers = false;
	uint32_t rebuildStartIndex = 255;

	// If layers are not built, or cube face changes, rebuild starting from root level
	if (m_planetLODLayers.size() == 0 || m_prevCubeFace != cubeFace)
	{
		rebuildStartIndex = 0;
		rebuildLODLayers = true;
	}
	else
	{
		// If more levels are required at this frame, rebuild starting from the end of existing layers
		if (m_prevLevel < level)
		{
			rebuildStartIndex = m_prevLevel + 1;
			rebuildLODLayers = true;
		}

		uint64_t prevValidBinaryU = m_prevBinaryCoord.x >> (fractionBits - level);
		uint64_t prevValidBinaryV = m_prevBinaryCoord.y >> (fractionBits - level);
		uint64_t currValidBinaryU = binaryCoord.x >> (fractionBits - level);
		uint64_t currValidBinaryV = binaryCoord.y >> (fractionBits - level);

		// If one or both of the binary coordinate between frames are different, pick a level that is nearest to root
		if (prevValidBinaryU != currValidBinaryU)
		{
			for (uint32_t i = 0; i < level; i++)
			{
				if ((prevValidBinaryU & (1ull << (level - 1 - i))) != (currValidBinaryU & (1ull << (level - 1 - i))))
				{
					rebuildStartIndex = ((i + 1) < rebuildStartIndex) ? (i + 1) : rebuildStartIndex;
					break;
				}
			}
			rebuildLODLayers = true;
		}
		if (prevValidBinaryV != currValidBinaryV)
		{
			for (uint32_t i = 0; i < level; i++)
			{
				if ((prevValidBinaryV & (1ull << (level - 1 - i))) != (currValidBinaryV & (1ull << (level - 1 - i))))
				{
					rebuildStartIndex = ((i + 1) < rebuildStartIndex) ? (i + 1) : rebuildStartIndex;
					break;
				}
			}
			rebuildLODLayers = true;
		}
	}

	std::shared_ptr<PlanetLODLayer> pLayer = nullptr;
	if (rebuildLODLayers)
	{
		for (uint32_t i = rebuildStartIndex; i <= level; i++)
		{
			if ((uint32_t)m_planetLODLayers.size() == i)
				m_planetLODLayers.push_back(PlanetLODLayer::Create());

			if (i != 0)
				pLayer = m_planetLODLayers[i - 1];

			m_planetLODLayers[i]->BuildupLayer(cubeFace, i, m_planetRadius, binaryCoord, pLayer);
		}
	}

	m_prevBinaryCoord = binaryCoord;
	m_prevCubeFace = cubeFace;
	m_prevLevel = level;

	// Process frustum culling
	for (uint32_t i = 0; i < (uint32_t)m_planetLODLayers.size(); i++)
	{
		m_planetLODLayers[i]->ProcessFrutumCulling(m_cameraFrustumLocal);
	}

	pLayer = nullptr;
	for (int32_t i = 0; i < (int32_t)UniformData::GetInstance()->GetGlobalUniforms()->GetMaximumRenderableLODLevel(); i++)
	{
		int32_t layer = (int32_t)level - i;
		if (layer < 0)
			break;

		if (layer != level)
			pLayer = m_planetLODLayers[layer + 1];

		m_planetLODLayers[layer]->PrepareGeometry(m_planetSpaceCameraPosition, m_planetRadius, layer, pLayer, pOutputTriangles);
	}
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

	NewPlanetLODMethod(pTriangles);
	uint32_t updatedSize = (uint32_t)((uint8_t*)pTriangles - startPtr);

	PlanetGeoDataManager::GetInstance()->FinishDataUpdate(updatedSize);

	if (m_pMeshRenderer != nullptr)
	{
		m_pMeshRenderer->SetStartInstance(offsetInBytes / sizeof(Triangle));
		m_pMeshRenderer->SetInstanceCount(updatedSize / sizeof(Triangle));
		m_pMeshRenderer->SetUtilityIndex(m_chunkIndex);
	}
}