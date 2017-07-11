#pragma once

#include "Buffer.h"

class IndexBuffer : public Buffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		const std::shared_ptr<IndexBuffer>& pSelf,
		uint32_t numBytes,
		VkIndexType type);

public:
	VkIndexType GetType() const { return m_type; }
	uint32_t GetCount() const { return m_count; }

public:
	static std::shared_ptr<IndexBuffer> Create(const std::shared_ptr<Device>& pDevice,
		uint32_t numBytes,
		VkIndexType type);

protected:
	VkIndexType m_type;
	uint32_t	m_count;
};