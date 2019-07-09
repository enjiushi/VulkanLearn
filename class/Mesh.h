#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../vulkan/DeviceObjectBase.h"
#include <string>
#include "../common/Enums.h"
#include "scene.h"

class SharedVertexBuffer;
class SharedIndexBuffer;
class CommandBuffer;

class Mesh : public SelfRefBase<Mesh>
{
public:
	static std::shared_ptr<Mesh> Create(const aiMesh* pMesh, uint32_t argumentedVertexFormat = 0);
	static std::shared_ptr<Mesh> Create(const std::string& filePath, uint32_t meshIndex, uint32_t argumentedVertexFormat = 0);
	static std::vector<std::shared_ptr<Mesh>> CreateMeshes(const std::string& filePath, uint32_t argumentedVertexFormat = 0);
	static std::shared_ptr<Mesh> Create
	(
		const void* pVertices, uint32_t verticesCount, uint32_t vertexFormat,
		const void* pIndices, uint32_t indicesCount, VkIndexType indexType
	);

public:
	std::shared_ptr<SharedVertexBuffer> GetVertexBuffer() const { return m_pVertexBuffer; }
	std::shared_ptr<SharedIndexBuffer> GetIndexBuffer() const { return m_pIndexBuffer; }
	uint32_t GetVertexFormat() const;
	uint32_t GetVertexBytes() const { return m_vertexBytes; }
	uint32_t GetVerticesCount() const { return m_verticesCount; }
	uint32_t GetMeshChunkIndex() const { return m_meshChunkIndex; }
	uint32_t GetMeshBoneChunkIndexOffset() const { return m_meshBoneChunkIndexOffset; }
	uint32_t ContainBoneData() const { return m_meshChunkIndex != -1; }
	uint32_t GetBoneCount() const { return m_boneCount; }
	void PrepareMeshData(const std::shared_ptr<CommandBuffer>& pCmdBuffer);
	void PrepareIndirectCmd(VkDrawIndexedIndirectCommand& cmd);

protected:
	bool Init
	(
		const std::shared_ptr<Mesh>& pSelf,
		const void* pVertices, uint32_t verticesCount, uint32_t vertexFormat,
		const void* pIndices, uint32_t indicesCount, VkIndexType indexType
	);

protected:
	std::shared_ptr<SharedVertexBuffer>	m_pVertexBuffer;
	std::shared_ptr<SharedIndexBuffer>	m_pIndexBuffer;
	uint32_t							m_verticesCount;
	uint32_t							m_vertexBytes;
	uint32_t							m_indicesCount;
	uint32_t							m_meshChunkIndex = -1;
	uint32_t							m_meshBoneChunkIndexOffset;
	uint32_t							m_boneCount;
};