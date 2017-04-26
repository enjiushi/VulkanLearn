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
};