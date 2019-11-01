#pragma once

#include "SharedBuffer.h"

class SharedIndexBuffer : public SharedBuffer
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		const std::shared_ptr<SharedIndexBuffer>& pSelf,
		uint32_t numBytes,
		VkIndexType type);

public:
	static std::shared_ptr<SharedIndexBuffer> Create(const std::shared_ptr<Device>& pDevice,
		uint32_t numBytes,
		VkIndexType type);

public:
	VkIndexType GetType() const { return m_type; }
	uint32_t GetCount() const { return m_count; }

protected:
	std::shared_ptr<BufferKey>	AcquireBuffer(uint32_t numBytes) override;

protected:
	VkIndexType m_type;
	uint32_t	m_count;
};