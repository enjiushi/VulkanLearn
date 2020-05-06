#include "StagingBufferManager.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include <algorithm>
#include "Queue.h"
#include "CommandBuffer.h"
#include "PerFrameResource.h"

bool StagingBufferManager::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<StagingBufferManager>& pSelf)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_pStagingBufferPool = StagingBuffer::Create(pDevice, STAGING_BUFFER_INC);

	return true;
}

std::shared_ptr<StagingBufferManager> StagingBufferManager::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<StagingBufferManager> pStagingBufferMgr = std::make_shared<StagingBufferManager>();
	if (pStagingBufferMgr.get() && pStagingBufferMgr->Init(pDevice, pStagingBufferMgr))
		return pStagingBufferMgr;
	return nullptr;
}

void StagingBufferManager::FlushDataMainThread()
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadGraphicPool()->AllocatePrimaryCommandBuffer();

	pCmdBuffer->StartPrimaryRecording();

	// Copy each chunk to dst buffer
	std::for_each(m_pendingUpdateBuffer.begin(), m_pendingUpdateBuffer.end(), [&](const PendingBufferInfo& info)
	{
		VkBufferCopy copy = {};
		copy.dstOffset = info.dstOffset;
		copy.srcOffset = info.srcOffset;
		copy.size = info.numBytes;
		pCmdBuffer->CopyBuffer(m_pStagingBufferPool, info.pBuffer, { copy });
	});

	pCmdBuffer->EndPrimaryRecording();

	GlobalGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);

	m_pendingUpdateBuffer.clear();
	m_usedNumBytes = 0;
}

void StagingBufferManager::RecordDataFlush(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	// Copy each chunk to dst buffer
	std::for_each(m_pendingUpdateBuffer.begin(), m_pendingUpdateBuffer.end(), [&](const PendingBufferInfo& info)
	{
		VkBufferCopy copy = {};
		copy.dstOffset = info.dstOffset;
		copy.srcOffset = info.srcOffset;
		copy.size = info.numBytes;
		pCmdBuffer->CopyBuffer(m_pStagingBufferPool, info.pBuffer, { copy });
	});

	m_pendingUpdateBuffer.clear();
	m_usedNumBytes = 0;
}

void StagingBufferManager::UpdateByteStream(const std::shared_ptr<BufferBase>& pBuffer, const void* pData, uint32_t offset, uint32_t numBytes)
{
	uint32_t currentOffset = m_usedNumBytes;
	m_pendingUpdateBuffer.push_back({ pBuffer, offset, currentOffset, numBytes });
	m_usedNumBytes += numBytes;

	if (m_usedNumBytes > m_pStagingBufferPool->GetBufferInfo().size)
	{
		// Acquire a bigger chunck of memory for staging buffer
		// Copy data into new staging buffer
		// Delete old staging buffer
		assert(false);
	}

	m_pStagingBufferPool->UpdateByteStream(pData, currentOffset, numBytes);
}