#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../vulkan/DeviceObjectBase.h"
#include <string>

class SharedVertexBuffer;
class SharedIndexBuffer;
class CommandBuffer;

class Mesh : public SelfRefBase<Mesh>
{
public:
	enum VertexAttribFlag
	{
		VAFPosition,
		VAFNormal,
		VAFColor,
		VAFTexCoord,
		VAFTangent
	};

public:
	static std::shared_ptr<Mesh> Create(const std::string& filePath);
	static std::shared_ptr<Mesh> Create
	(
		const void* pVertices, uint32_t verticesCount, uint32_t vertexAttribFlag,
		const void* pIndices, uint32_t indicesCount, VkIndexType indexType
	);

	static uint32_t GetVertexBytes(uint32_t vertexAttribFlag);
	static uint32_t GetIndexBytes(VkIndexType indexType);

public:
	std::shared_ptr<SharedVertexBuffer> GetVertexBuffer() const { return m_pVertexBuffer; }
	std::shared_ptr<SharedIndexBuffer> GetIndexBuffer() const { return m_pIndexBuffer; }
	uint32_t GetVertexAttribFlag() const { return m_vertexAttribFlag; }
	uint32_t GetVertexBytes() const { return m_vertexBytes; }
	uint32_t GetVerticesCount() const { return m_verticesCount; }
	void PrepareMeshData(const std::shared_ptr<CommandBuffer>& pCmdBuffer);

protected:
	bool Init
	(
		const void* pVertices, uint32_t verticesCount, uint32_t vertexAttribFlag,
		const void* pIndices, uint32_t indicesCount, VkIndexType indexType
	);

protected:
	std::shared_ptr<SharedVertexBuffer>	m_pVertexBuffer;
	std::shared_ptr<SharedIndexBuffer>	m_pIndexBuffer;
	uint32_t							m_vertexAttribFlag;
	uint32_t							m_verticesCount;
	uint32_t							m_vertexBytes;
};