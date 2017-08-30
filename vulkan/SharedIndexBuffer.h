#pragma once

#include "IndexBuffer.h"
#include "SharedBufferManager.h"

class SharedIndexBuffer : public IndexBuffer
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		const std::shared_ptr<IndexBuffer>& pSelf,
		uint32_t numBytes,
		VkIndexType type);

public:
	static std::shared_ptr<SharedIndexBuffer> Create(const std::shared_ptr<Device>& pDevice,
		uint32_t numBytes,
		VkIndexType type);

	VkBuffer GetDeviceHandle() const override { return m_pBufferKey->GetSharedBufferMgr()->GetBuffer()->GetDeviceHandle(); }
	void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes) override;
	uint32_t GetBufferOffset() const override;

protected:
	std::shared_ptr<BufferKey> m_pBufferKey;
};