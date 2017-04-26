#pragma once

#include "Buffer.h"

class StagingBuffer : public Buffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		uint32_t numBytes, 
		const void* pData = nullptr);

public:
	static std::shared_ptr<StagingBuffer> Create(const std::shared_ptr<Device>& pDevice,
		uint32_t numBytes,
		const void* pData = nullptr);

public:
	void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes) override;
};