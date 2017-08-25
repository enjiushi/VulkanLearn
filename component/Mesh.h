#pragma once
#include "../Base/BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../vulkan/DeviceObjectBase.h"
#include <string>

class VertexBuffer;
class IndexBuffer;

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
	static uint32_t GetVertexBytes(uint32_t vertexAttribFlag);
	static uint32_t GetIndexBytes(VkIndexType indexType);

public:
	std::shared_ptr<VertexBuffer> GetVertexBuffer() const { return m_pVertexBuffer; }
	std::shared_ptr<IndexBuffer> GetIndexBuffer() const { return m_pIndexBuffer; }
	uint32_t GetVertexAttribFlag() const { return m_vertexAttribFlag; }
	uint32_t GetVertexBytes() const { return m_vertexBytes; }
	uint32_t GetVerticesCount() const { return m_verticesCount; }

protected:
	bool Init
	(
		const void* pVertices, uint32_t verticesCount, uint32_t vertexAttribFlag,
		const void* pIndices, uint32_t indicesCount, VkIndexType indexType
	);

protected:
	std::shared_ptr<VertexBuffer>	m_pVertexBuffer;
	std::shared_ptr<IndexBuffer>	m_pIndexBuffer;
	uint32_t						m_vertexAttribFlag;
	uint32_t						m_verticesCount;
	uint32_t						m_vertexBytes;
};