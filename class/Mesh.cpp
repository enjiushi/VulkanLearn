#include "Mesh.h"
#include "../vulkan/SharedVertexBuffer.h"
#include "../vulkan/SharedIndexBuffer.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/CommandBuffer.h"
#include "../Maths/AssimpDataConverter.h"
#include "UniformData.h"
#include "Importer.hpp"
#include "postprocess.h"
#include <string>
#include "../common/Util.h"
#include <codecvt>
#include <locale>

bool Mesh::Init
(
	const std::shared_ptr<Mesh>& pSelf,
	const void* pVertices, uint32_t verticesCount, uint32_t vertexFormat,
	const void* pIndices, uint32_t indicesCount, VkIndexType indexType
)
{
	if (!SelfRefBase<Mesh>::Init(pSelf))
		return false;

	m_vertexBytes = ::GetVertexBytes(vertexFormat);
	m_verticesCount = verticesCount;
	m_indicesCount = indicesCount;

	m_pVertexBuffer = SharedVertexBuffer::Create(GetDevice(), m_verticesCount * m_vertexBytes, vertexFormat);
	m_pVertexBuffer->UpdateByteStream(pVertices, 0, m_verticesCount * m_vertexBytes);
	m_pIndexBuffer = SharedIndexBuffer::Create(GetDevice(), indicesCount * GetIndexBytes(indexType), indexType);
	m_pIndexBuffer->UpdateByteStream(pIndices, 0, indicesCount * GetIndexBytes(indexType));

	return true;
}

std::shared_ptr<Mesh> Mesh::Create
(
	const void* pVertices, uint32_t verticesCount, uint32_t vertexFormat,
	const void* pIndices, uint32_t indicesCount, VkIndexType indexType
)
{
	std::shared_ptr<Mesh> pRetMesh = std::make_shared<Mesh>();
	if (pRetMesh.get() && pRetMesh->Init
	(
		pRetMesh,
		pVertices, verticesCount, vertexFormat,
		pIndices, indicesCount, indexType
	))
		return pRetMesh;
	return nullptr;
}

std::shared_ptr<Mesh> Mesh::Create(const std::string& filePath, uint32_t meshIndex, uint32_t argumentedVertexFormat)
{
	Assimp::Importer imp;
	const aiScene* pScene = imp.ReadFile(filePath.c_str(), aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	ASSERTION(pScene != nullptr);

	return Create(pScene->mMeshes[meshIndex], argumentedVertexFormat);
}

std::vector<std::shared_ptr<Mesh>> Mesh::CreateMeshes(const std::string& filePath, uint32_t argumentedVertexFormat)
{
	Assimp::Importer imp;
	const aiScene* pScene = nullptr;
	pScene = imp.ReadFile(filePath.c_str(), aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	ASSERTION(pScene != nullptr);

	std::vector<std::shared_ptr<Mesh>> meshes;
	for (int32_t i = 0; i < pScene->mNumMeshes; i++)
	{
		std::shared_ptr<Mesh> pMesh = Create(pScene->mMeshes[i], argumentedVertexFormat);
		if (pMesh == nullptr)
			return {};

		meshes.push_back(pMesh);
	}
	return meshes;
}

std::shared_ptr<Mesh> Mesh::Create(const aiMesh* pMesh, uint32_t argumentedVertexFormat)
{
	uint32_t vertexFormat = 0;

	uint32_t vertexSize = 0;
	if (pMesh->HasPositions())
	{
		vertexSize += 3 * sizeof(float);
		vertexFormat |= (1 << VAFPosition);
	}
	if (pMesh->HasNormals())
	{
		vertexSize += 3 * sizeof(float);
		vertexFormat |= (1 << VAFNormal);
	}
	//FIXME: hard-coded index 0 here, we don't have more than 1 color for now
	if (pMesh->HasVertexColors(0))
	{
		vertexSize += 4 * sizeof(float);
		vertexFormat |= (1 << VAFColor);
	}
	//FIXME: hard-coded index 0 here, we don't have more than 1 texture coord for now
	if (pMesh->HasTextureCoords(0))
	{
		vertexSize += 2 * sizeof(float);
		vertexFormat |= (1 << VAFTexCoord);
	}
	if (pMesh->HasTangentsAndBitangents())
	{
		vertexSize += 3 * sizeof(float);
		vertexFormat |= (1 << VAFTangent);
	}
	if (pMesh->HasBones())
	{
		vertexSize += 4 * sizeof(float);	// Bone weight
		vertexSize += 4 * sizeof(uint8_t);	// Bone index
		vertexFormat |= (1 << VAFBone);
	}

	if (vertexFormat != argumentedVertexFormat && argumentedVertexFormat != 0)
		return nullptr;

	float* pVertices = new float[pMesh->mNumVertices * vertexSize / sizeof(float)];
	memset(pVertices, 0, pMesh->mNumVertices * vertexSize);
	uint32_t verticesNumBytes = pMesh->mNumVertices * vertexSize;
	uint32_t count = 0;

	for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
	{
		uint32_t offset = i * vertexSize / sizeof(float);
		count = 0;
		if (vertexFormat & (1 << VAFPosition))
		{
			pVertices[offset] = pMesh->mVertices[i].x;
			pVertices[offset + 1] = pMesh->mVertices[i].y;
			pVertices[offset + 2] = pMesh->mVertices[i].z;
			count += 3;
		}
		if (vertexFormat & (1 << VAFNormal))
		{
			pVertices[offset + count] = pMesh->mNormals[i].x;
			pVertices[offset + count + 1] = pMesh->mNormals[i].y;
			pVertices[offset + count + 2] = pMesh->mNormals[i].z;
			count += 3;
		}
		if (vertexFormat & (1 << VAFColor))
		{
			pVertices[offset + count] = pMesh->mColors[i][0].r;
			pVertices[offset + count + 1] = pMesh->mColors[i][0].g;
			pVertices[offset + count + 2] = pMesh->mColors[i][0].b;
			pVertices[offset + count + 3] = pMesh->mColors[i][0].a;
			count += 4;
		}
		if (vertexFormat & (1 << VAFTexCoord))
		{
			pVertices[offset + count] = pMesh->mTextureCoords[0][i].x;
			pVertices[offset + count + 1] = pMesh->mTextureCoords[0][i].y;
			count += 2;
		}
		if (vertexFormat & (1 << VAFTangent))
		{
			pVertices[offset + count] = pMesh->mTangents[i].x;
			pVertices[offset + count + 1] = pMesh->mTangents[i].y;
			pVertices[offset + count + 2] = pMesh->mTangents[i].z;
			count += 3;
		}
	}

	if (vertexFormat & (1 << VAFBone))
	{
		uint32_t vertexSizeInFloats = vertexSize / sizeof(float);

		uint8_t* pOffsets = new uint8_t[pMesh->mNumVertices];
		memset(pOffsets, 0, pMesh->mNumVertices);
		for (uint32_t i = 0; i < pMesh->mNumBones; i++)
		{
			for (uint32_t j = 0; j < pMesh->mBones[i]->mNumWeights; j++)
			{
				float boneWeight = pMesh->mBones[i]->mWeights[j].mWeight;
				int32_t vertexID = pMesh->mBones[i]->mWeights[j].mVertexId;

				ASSERTION(pOffsets[vertexID] <= 4);

				pVertices[vertexSizeInFloats * vertexID + count + pOffsets[vertexID]] = boneWeight;

				uint8_t* pBoneIndex = (uint8_t*)(&pVertices[vertexSizeInFloats * vertexID + count + 4]);
				pBoneIndex[pOffsets[vertexID]] = i;

				pOffsets[vertexID]++;
			}
		}
		delete[] pOffsets;
	}

	uint32_t* pIndices = new uint32_t[pMesh->mNumFaces * 3];
	uint32_t indicesNumBytes = pMesh->mNumFaces * 3 * sizeof(uint32_t);
	for (size_t i = 0; i < pMesh->mNumFaces; i++)
	{
		pIndices[i * 3] = pMesh->mFaces[i].mIndices[0];
		pIndices[i * 3 + 1] = pMesh->mFaces[i].mIndices[1];
		pIndices[i * 3 + 2] = pMesh->mFaces[i].mIndices[2];
	}

	std::shared_ptr<Mesh> pRetMesh = std::make_shared<Mesh>();
	if (pRetMesh.get() && pRetMesh->Init
	(
		pRetMesh,
		pVertices, pMesh->mNumVertices, vertexFormat,
		pIndices, pMesh->mNumFaces * 3, VK_INDEX_TYPE_UINT32
	))
	{
		pRetMesh->m_boneCount = pMesh->mNumBones;

		if (pMesh->mNumBones)
			pRetMesh->m_meshBoneChunkIndexOffset = UniformData::GetInstance()->GetPerBoneIndirectUniforms()->AllocateConsecutiveChunks(pMesh->mNumBones);

		for (uint32_t i = 0; i < pMesh->mNumBones; i++)
		{
			DualQuaternionf dq = AssimpDataConverter::AcquireDualQuaternion(pMesh->mBones[i]->mOffsetMatrix);
			UniformData::GetInstance()->GetPerBoneIndirectUniforms()->SetBoneTransform(pRetMesh->m_meshBoneChunkIndexOffset, std::hash<std::wstring>()(std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(pMesh->mBones[i]->mName.C_Str())), dq.DoublePrecision());
		}

		pRetMesh->m_meshChunkIndex = UniformData::GetInstance()->GetPerMeshUniforms()->AllocatePerObjectChunk();
		UniformData::GetInstance()->GetPerMeshUniforms()->SetBoneChunkIndexOffset(pRetMesh->m_meshChunkIndex, pRetMesh->m_meshBoneChunkIndexOffset);

		delete[] pVertices;
		delete[] pIndices;
		return pRetMesh;
	}

	delete[] pVertices;
	delete[] pIndices;
	return nullptr;
}

uint32_t Mesh::GetVertexFormat() const
{ 
	return m_pVertexBuffer->GetVertexFormat();
}

void Mesh::PrepareIndirectCmd(VkDrawIndexedIndirectCommand& cmd)
{
	// FIXME: No instanced rendering for now, hard coded
	cmd.firstInstance = 0;
	cmd.instanceCount = 1;

	cmd.vertexOffset = GetVertexBuffer()->GetBufferOffset() / m_vertexBytes;
	cmd.firstIndex = GetIndexBuffer()->GetBufferOffset() / GetIndexBytes(GetIndexBuffer()->GetType());
	cmd.indexCount = m_indicesCount;
}