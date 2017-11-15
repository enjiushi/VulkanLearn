#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../vulkan/DeviceObjectBase.h"
#include <string>
#include "Enums.h"

class SharedVertexBuffer;
class SharedIndexBuffer;
class CommandBuffer;

class Mesh : public SelfRefBase<Mesh>
{
public:
	static std::shared_ptr<Mesh> Create(const std::string& filePath);
	static std::shared_ptr<Mesh> Create
	(
		const void* pVertices, uint32_t verticesCount, uint32_t vertexAttribFlag,
		const void* pIndices, uint32_t indicesCount, VkIndexType indexType
	);

public:
	std::shared_ptr<SharedVertexBuffer> GetVertexBuffer() const { return m_pVertexBuffer; }
	std::shared_ptr<SharedIndexBuffer> GetIndexBuffer() const { return m_pIndexBuffer; }
	uint32_t GetVertexFormat() const;
	uint32_t GetVertexBytes() const { return m_vertexBytes; }
	uint32_t GetVerticesCount() const { return m_verticesCount; }
	void PrepareMeshData(const std::shared_ptr<CommandBuffer>& pCmdBuffer);
	void PrepareIndirectCmd(VkDrawIndexedIndirectCommand& cmd);

protected:
	bool Init
	(
		const void* pVertices, uint32_t verticesCount, uint32_t vertexAttribFlag,
		const void* pIndices, uint32_t indicesCount, VkIndexType indexType
	);

protected:
	std::shared_ptr<SharedVertexBuffer>	m_pVertexBuffer;
	std::shared_ptr<SharedIndexBuffer>	m_pIndexBuffer;
	uint32_t							m_verticesCount;
	uint32_t							m_vertexBytes;
	uint32_t							m_indicesCount;
};