#pragma once

#include "SharedBuffer.h"

class SharedVertexBuffer : public SharedBuffer
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice,
		const std::shared_ptr<SharedVertexBuffer>& pSelf,
		uint32_t numBytes,
		uint32_t vertexFormat);

public:
	uint32_t GetNumVertices() const { return m_numVertices; }
	uint32_t GetVertexFormat() const { return m_vertexFormat; }

public:
	static std::shared_ptr<SharedVertexBuffer> Create(const std::shared_ptr<Device>& pDevice,
		uint32_t numBytes,
		uint32_t vertexFormat);

protected:
	std::shared_ptr<BufferKey>	AcquireBuffer(uint32_t numBytes) override;

protected:
	uint32_t					m_numVertices;
	uint32_t					m_vertexFormat;
};