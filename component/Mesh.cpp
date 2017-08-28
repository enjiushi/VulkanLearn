#include "Mesh.h"
#include "../vulkan/VertexBuffer.h"
#include "../vulkan/IndexBuffer.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "Importer.hpp"
#include "scene.h"
#include "postprocess.h"
#include <string>

uint32_t Mesh::GetVertexBytes(uint32_t vertexAttribFlag)
{
	uint32_t vertexByte = 0;
	if (vertexAttribFlag & (1 << VAFPosition))
	{
		vertexByte += 3 * sizeof(float);
	}
	if (vertexAttribFlag & (1 << VAFNormal))
	{
		vertexByte += 3 * sizeof(float);
	}
	if (vertexAttribFlag & (1 << VAFColor))
	{
		vertexByte += 4 * sizeof(float);
	}
	if (vertexAttribFlag & (1 << VAFTexCoord))
	{
		vertexByte += 2 * sizeof(float);
	}
	if (vertexAttribFlag & (1 << VAFTangent))
	{
		vertexByte += 6 * sizeof(float);
	}
	return vertexByte;
}

uint32_t Mesh::GetIndexBytes(VkIndexType indexType)
{
	switch (indexType)
	{
	case VK_INDEX_TYPE_UINT16: return 2;
	case VK_INDEX_TYPE_UINT32: return 4;
	default: ASSERTION(false);
	}
	return 0;
}

bool Mesh::Init
(
	const void* pVertices, uint32_t verticesCount, uint32_t vertexAttribFlag,
	const void* pIndices, uint32_t indicesCount, VkIndexType indexType
)
{
	m_vertexBytes = GetVertexBytes(vertexAttribFlag);
	m_vertexAttribFlag = vertexAttribFlag;
	m_verticesCount = verticesCount;

	//Binding and attributes information
	VkVertexInputBindingDescription bindingDesc = {};
	bindingDesc.binding = 0;
	bindingDesc.stride = m_vertexBytes;
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> attribDesc;

	uint32_t offset = 0;

	if (vertexAttribFlag & (1 << VAFPosition))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = 0;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFPosition;
		attrib.offset = offset;
		offset += sizeof(float) * 3;
		attribDesc.push_back(attrib);
	}
	if (vertexAttribFlag & (1 << VAFNormal))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = 0;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFNormal;
		attrib.offset = offset;
		offset += sizeof(float) * 3;
		attribDesc.push_back(attrib);
	}
	if (vertexAttribFlag & (1 << VAFColor))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = 0;
		attrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attrib.location = VAFColor;
		attrib.offset = offset;
		offset += sizeof(float) * 4;
		attribDesc.push_back(attrib);
	}
	if (vertexAttribFlag & (1 << VAFTexCoord))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = 0;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFTexCoord;
		attrib.offset = offset;
		offset += sizeof(float) * 2;
		attribDesc.push_back(attrib);
	}
	if (vertexAttribFlag & (1 << VAFTangent))
	{
		VkVertexInputAttributeDescription attrib = {};
		attrib.binding = 0;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFTangent;
		attrib.offset = offset;
		offset += sizeof(float) * 3;
		attribDesc.push_back(attrib);

		attrib = {};
		attrib.binding = 0;
		attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
		attrib.location = VAFTangent + 1;
		attrib.offset = offset;
		offset += sizeof(float) * 3;
		attribDesc.push_back(attrib);
	}

	m_pVertexBuffer = VertexBuffer::Create(GetDevice(), m_verticesCount * m_vertexBytes, bindingDesc, attribDesc);
	m_pVertexBuffer->UpdateByteStream(pVertices, 0, m_verticesCount * m_vertexBytes);
	m_pIndexBuffer = IndexBuffer::Create(GetDevice(), indicesCount * GetIndexBytes(indexType), indexType);
	m_pIndexBuffer->UpdateByteStream(pIndices, 0, indicesCount * GetIndexBytes(indexType));

	return true;
}

std::shared_ptr<Mesh> Mesh::Create
(
	const void* pVertices, uint32_t verticesCount, uint32_t vertexAttribFlag,
	const void* pIndices, uint32_t indicesCount, VkIndexType indexType
)
{
	std::shared_ptr<Mesh> pRetMesh = std::make_shared<Mesh>();
	if (pRetMesh.get() && pRetMesh->Init
	(
		pVertices, verticesCount, vertexAttribFlag,
		pIndices, indicesCount, indexType
	))
		return pRetMesh;
	return nullptr;
}

std::shared_ptr<Mesh> Mesh::Create(const std::string& filePath)
{
	Assimp::Importer imp;
	const aiScene* pScene = nullptr;
	pScene = imp.ReadFile(filePath.c_str(), aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);
	ASSERTION(pScene != nullptr);

	// FIXME: Now scene file is used as single mesh, it shouldn't be however, some class like "scene" should load it and dispatch mesh data to this class, in future
	aiMesh* pMesh = pScene->mMeshes[0];

	uint32_t vertexAttribFlag = 0;

	uint32_t vertexSize = 0;
	if (pMesh->HasPositions())
	{
		vertexSize += 3 * sizeof(float);
		vertexAttribFlag |= (1 << VAFPosition);
	}
	if (pMesh->HasNormals())
	{
		vertexSize += 3 * sizeof(float);
		vertexAttribFlag |= (1 << VAFNormal);
	}
	//FIXME: hard-coded index 0 here, we don't have more than 1 color for now
	if (pMesh->HasVertexColors(0))
	{
		vertexSize += 4 * sizeof(float);
		vertexAttribFlag |= (1 << VAFColor);
	}
	//FIXME: hard-coded index 0 here, we don't have more than 1 texture coord for now
	if (pMesh->HasTextureCoords(0))
	{
		vertexSize += 2 * sizeof(float);
		vertexAttribFlag |= (1 << VAFTexCoord);
	}
	if (pMesh->HasTangentsAndBitangents())
	{
		vertexSize += 6 * sizeof(float);
		vertexAttribFlag |= (1 << VAFTangent);
	}

	float* pVertices = new float[pMesh->mNumVertices * vertexSize / sizeof(float)];
	uint32_t verticesNumBytes = pMesh->mNumVertices * vertexSize;
	uint32_t count = 0;

	for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
	{
		uint32_t offset = i * vertexSize / sizeof(float);
		count = 0;
		if (vertexAttribFlag & (1 << VAFPosition))
		{
			pVertices[offset] = pMesh->mVertices[i].x;
			pVertices[offset + 1] = pMesh->mVertices[i].y;
			pVertices[offset + 2] = pMesh->mVertices[i].z;
			count += 3;
		}
		if (vertexAttribFlag & (1 << VAFNormal))
		{
			pVertices[offset + count] = pMesh->mNormals[i].x;
			pVertices[offset + count + 1] = pMesh->mNormals[i].y;
			pVertices[offset + count + 2] = pMesh->mNormals[i].z;
			count += 3;
		}
		if (vertexAttribFlag & (1 << VAFColor))
		{
			pVertices[offset + count] = pMesh->mColors[i][0].r;
			pVertices[offset + count + 1] = pMesh->mColors[i][0].g;
			pVertices[offset + count + 2] = pMesh->mColors[i][0].b;
			pVertices[offset + count + 3] = pMesh->mColors[i][0].a;
			count += 4;
		}
		if (vertexAttribFlag & (1 << VAFTexCoord))
		{
			pVertices[offset + count] = pMesh->mTextureCoords[0][i].x;
			pVertices[offset + count + 1] = pMesh->mTextureCoords[0][i].y;
			count += 2;
		}
		if (vertexAttribFlag & (1 << VAFTangent))
		{
			pVertices[offset + count] = pMesh->mTangents[i].x;
			pVertices[offset + count + 1] = pMesh->mTangents[i].y;
			pVertices[offset + count + 2] = pMesh->mTangents[i].z;
			pVertices[offset + count + 3] = pMesh->mBitangents[i].x;
			pVertices[offset + count + 4] = pMesh->mBitangents[i].y;
			pVertices[offset + count + 5] = pMesh->mBitangents[i].z;
			count += 6;
		}
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
		pVertices, pMesh->mNumVertices, vertexAttribFlag,
		pIndices, pMesh->mNumFaces * 3, VK_INDEX_TYPE_UINT32
	))
		return pRetMesh;
	return nullptr;
}