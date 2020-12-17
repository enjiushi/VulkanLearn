#pragma once

#include "Buffer.h"

class StagingBuffer : public Buffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice,  const std::shared_ptr<StagingBuffer>& pSelf, uint32_t numBytes, VkAccessFlags accessFlags);

public:
	static std::shared_ptr<StagingBuffer> Create(const std::shared_ptr<Device>& pDevice, uint32_t numBytes);
	static std::shared_ptr<StagingBuffer> CreateReadableStagingBuffer(const std::shared_ptr<Device>& pDevice, uint32_t numBytes);

public:
	void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes) override;
	void* GetDataPtr() const { return m_pData; }

private:
	void*	m_pData;
};