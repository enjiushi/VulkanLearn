#include "StagingBufferManager.h"

bool StagingBufferManager::Init(const std::shared_ptr<Device>& pDevice)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	m_pStagingBufferPool = StagingBuffer::Create(pDevice, STAGING_BUFFER_INC);
	return true;
}

std::shared_ptr<StagingBufferManager> StagingBufferManager::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<StagingBufferManager> pStagingBufferMgr = std::make_shared<StagingBufferManager>();
	if (pStagingBufferMgr.get() && pStagingBufferMgr->Init(pDevice))
		return pStagingBufferMgr;
	return nullptr;
}

void StagingBufferManager::FlushData()
{
	
}

void StagingBufferManager::UpdateByteStream(const Buffer* pBuffer, const void* pData, uint32_t offset, uint32_t numBytes)
{
	PendingBufferInfo pendingInfo = { pBuffer, offset, numBytes };
	m_pendingUpdateBuffer.push_back(pendingInfo);
	m_usedNumBytes += numBytes;

	if (m_usedNumBytes > m_pStagingBufferPool->GetBufferInfo().size)
	{
		// Acquire a bigger chunck of memory for staging buffer
		// Copy data into new staging buffer
		// Delete old staging buffer
		assert(false);
	}

	m_pStagingBufferPool->UpdateByteStream(pData, offset, numBytes);
}