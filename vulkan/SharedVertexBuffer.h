#pragma once

#include "VertexBuffer.h"
#include "SharedBufferManager.h"

class SharedVertexBuffer : public VertexBuffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice,
		const std::shared_ptr<SharedVertexBuffer>& pSelf,
		uint32_t numBytes,
		uint32_t vertexFormat);

public:
	uint32_t GetNumVertices() const { return m_numVertices; }
	VkBuffer GetDeviceHandle() const override { return m_pBufferKey->GetSharedBufferMgr()->GetBuffer()->GetDeviceHandle(); }
	void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes) override;
	uint32_t GetBufferOffset() const override;

public:
	static std::shared_ptr<SharedVertexBuffer> Create(const std::shared_ptr<Device>& pDevice,
		uint32_t numBytes,
		uint32_t vertexFormat);

protected:
	std::shared_ptr<BufferKey>	m_pBufferKey;
};