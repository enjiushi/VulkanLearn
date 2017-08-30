#pragma once

#include "StagingBuffer.h"

class PerFrameResource;
class CommandBuffer;

class StagingBufferManager : public DeviceObjectBase<StagingBufferManager>
{
	typedef struct _PendingBufferInfo
	{
		std::shared_ptr<Buffer> pBuffer;
		uint32_t dstOffset;
		uint32_t srcOffset;
		uint32_t numBytes;
	}PendingBufferInfo;

public:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<StagingBufferManager>& pSelf);

	static std::shared_ptr<StagingBufferManager> Create(const std::shared_ptr<Device>& pDevice);

public:
	void FlushDataMainThread();
	void RecordDataFlush(const std::shared_ptr<CommandBuffer>& pCmdBuffer);

protected:
	void UpdateByteStream(const std::shared_ptr<Buffer>& pBuffer, const void* pData, uint32_t offset, uint32_t numBytes);

protected:
	std::shared_ptr<StagingBuffer>	m_pStagingBufferPool;
	std::vector<PendingBufferInfo>	m_pendingUpdateBuffer;
	uint32_t						m_usedNumBytes = 0;
	const static uint32_t STAGING_BUFFER_INC = 1024 * 1024 * 8;
	friend class Buffer;
	friend class Image;
	friend class SharedBufferManager;
};