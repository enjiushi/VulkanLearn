#pragma once

#include "StagingBuffer.h"

class StagingBufferManager : public DeviceObjectBase
{
	typedef struct _PendingBufferInfo
	{
		const Buffer* pBuffer;
		uint32_t dstOffset;
		VkPipelineStageFlagBits dstStage;
		VkAccessFlags dstAccess;
		uint32_t srcOffset;
		uint32_t numBytes;
	}PendingBufferInfo;

public:
	bool Init(const std::shared_ptr<Device>& pDevice);

	static std::shared_ptr<StagingBufferManager> Create(const std::shared_ptr<Device>& pDevice);

public:
	void FlushData();

protected:
	void UpdateByteStream(const Buffer* pBuffer, const void* pData, uint32_t offset, uint32_t numBytes, VkPipelineStageFlagBits dstStage, VkAccessFlags dstAccess);

protected:
	std::shared_ptr<StagingBuffer>	m_pStagingBufferPool;
	std::vector<PendingBufferInfo>	m_pendingUpdateBuffer;
	uint32_t						m_usedNumBytes = 0;
	const static uint32_t STAGING_BUFFER_INC = 1024 * 1024 * 8;
	friend class Buffer;
};