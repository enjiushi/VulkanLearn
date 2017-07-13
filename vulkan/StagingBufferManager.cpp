#include "StagingBufferManager.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include <algorithm>
#include "Queue.h"
#include "CommandBuffer.h"

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

// FIXEME: this function should replace native device objects with wrapper class
void StagingBufferManager::FlushData()
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = GlobalObjects()->GetMainThreadCmdPool()->AllocatePrimaryCommandBuffer();

	
	CommandBuffer::BufferCopyCmdData data;
	data.preBarriers =
	{
		{
			m_pStagingBufferPool,
			VK_PIPELINE_STAGE_HOST_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_ACCESS_HOST_WRITE_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			0,
			m_usedNumBytes
		}
	};

	for (uint32_t i = 0; i < m_pendingUpdateBuffer.size(); i++)
	{
		VkBufferCopy copy = {};
		copy.dstOffset = m_pendingUpdateBuffer[i].dstOffset;
		copy.srcOffset = m_pendingUpdateBuffer[i].srcOffset;
		copy.size = m_pendingUpdateBuffer[i].numBytes;

		CommandBuffer::CopyData copyData;
		copyData.pSrcBuffer = m_pStagingBufferPool;
		copyData.copyData = copy;
		copyData.pDstBuffer = m_pendingUpdateBuffer[i].pBuffer;
		data.copyData.push_back(copyData);
	}

	for (uint32_t i = 0; i < m_pendingUpdateBuffer.size(); i++)
	{
		CommandBuffer::BarrierData barrierData;
		barrierData.srcStages	= VK_PIPELINE_STAGE_TRANSFER_BIT;
		barrierData.dstStages	= m_pendingUpdateBuffer[i].dstStage;
		barrierData.srcAccess	= VK_ACCESS_TRANSFER_WRITE_BIT;
		barrierData.dstAccess	= m_pendingUpdateBuffer[i].dstAccess;
		barrierData.pBuffer		= m_pendingUpdateBuffer[i].pBuffer;
		barrierData.offset		= m_pendingUpdateBuffer[i].dstOffset;
		barrierData.size		= m_pendingUpdateBuffer[i].numBytes;
		data.postBarriers.push_back(barrierData);
	}

	pCmdBuffer->PrepareBufferCopyCommands(data);

	GlobalObjects()->GetGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
	
	m_pendingUpdateBuffer.clear();
	m_usedNumBytes = 0;
	
}

void StagingBufferManager::UpdateByteStream(const std::shared_ptr<Buffer>& pBuffer, const void* pData, uint32_t offset, uint32_t numBytes, VkPipelineStageFlagBits dstStage, VkAccessFlags dstAccess)
{
	uint32_t currentOffset = m_usedNumBytes;
	m_pendingUpdateBuffer.push_back({ pBuffer, offset, dstStage, dstAccess, currentOffset, numBytes });
	m_usedNumBytes += numBytes;

	if (m_usedNumBytes > m_pStagingBufferPool->GetBufferInfo().size)
	{
		// Acquire a bigger chunck of memory for staging buffer
		// Copy data into new staging buffer
		// Delete old staging buffer
		assert(false);
	}

	m_pStagingBufferPool->UpdateByteStream(pData, currentOffset, numBytes, dstStage, dstAccess);
}