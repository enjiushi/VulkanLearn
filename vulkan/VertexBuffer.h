#pragma once

#include "Buffer.h"

class VertexBuffer : public Buffer
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice,
		const std::shared_ptr<VertexBuffer>& pSelf,
		uint32_t numBytes,
		uint32_t vertexFormat);

	void InitDesc(uint32_t numBytes, uint32_t vertexFormat);

public:
	uint32_t GetNumVertices() const { return m_numVertices; }
	uint32_t GetVertexFormat() const { return m_vertexFormat; }

public:
	static std::shared_ptr<VertexBuffer> Create(const std::shared_ptr<Device>& pDevice, 
		uint32_t numBytes,
		uint32_t vertexFormat);

protected:
	uint32_t										m_numVertices;
	uint32_t										m_vertexFormat;
};