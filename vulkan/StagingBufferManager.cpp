#include "StagingBufferManager.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include <algorithm>
#include "Queue.h"

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

// FIXEME: this function should replace native device objects with wrapper class
void StagingBufferManager::FlushData()
{
	// FIXME: Use native device objects here for now
	VkCommandBuffer cmdBuffer = GlobalDeviceObjects::GetInstance()->GetMainThreadCmdPool()->AllocateCommandBuffer();
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	VkBufferMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.buffer = m_pStagingBufferPool->GetDeviceHandle();
	barrier.offset = 0;
	barrier.size = m_usedNumBytes;

	vkCmdPipelineBarrier(cmdBuffer,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		1, &barrier,
		0, nullptr);

	// Copy each chunk to dst buffer
	std::for_each(m_pendingUpdateBuffer.begin(), m_pendingUpdateBuffer.end(), [&](const PendingBufferInfo& info)
	{
		VkBufferCopy copy = {};
		copy.dstOffset = info.dstOffset;
		copy.srcOffset = info.srcOffset;
		copy.size = info.numBytes;
		vkCmdCopyBuffer(cmdBuffer, m_pStagingBufferPool->GetDeviceHandle(), info.pBuffer->GetDeviceHandle(), 1, &copy);
	});

	// Create barriers ordered by pipeline stage
	std::map<VkPipelineStageFlagBits, std::vector<VkBufferMemoryBarrier>> barrierTable;
	std::for_each(m_pendingUpdateBuffer.begin(), m_pendingUpdateBuffer.end(), [&](const PendingBufferInfo& info)
	{
		VkBufferMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = info.dstAccess;
		barrier.buffer = info.pBuffer->GetDeviceHandle();
		barrier.offset = info.dstOffset;
		barrier.size = info.numBytes;

		barrierTable[info.dstStage].push_back(barrier);
	});

	// For each pipeline stage, apply barriers
	std::for_each(barrierTable.begin(), barrierTable.end(), [&](const std::pair<VkPipelineStageFlagBits, std::vector<VkBufferMemoryBarrier>>& pair)
	{
		vkCmdPipelineBarrier(cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			pair.first,
			0,
			0, nullptr,
			pair.second.size(), pair.second.data(),
			0, nullptr);
	});

	CHECK_VK_ERROR(vkEndCommandBuffer(cmdBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	CHECK_VK_ERROR(vkQueueSubmit(GlobalDeviceObjects::GetInstance()->GetGraphicQueue()->GetDeviceHandle(), 1, &submitInfo, nullptr));
	vkQueueWaitIdle(GlobalDeviceObjects::GetInstance()->GetGraphicQueue()->GetDeviceHandle());

	vkFreeCommandBuffers(m_pDevice->GetDeviceHandle(), GlobalDeviceObjects::GetInstance()->GetMainThreadCmdPool()->GetDeviceHandle(), 1, &cmdBuffer);

	m_pendingUpdateBuffer.clear();
}

void StagingBufferManager::UpdateByteStream(const Buffer* pBuffer, const void* pData, uint32_t offset, uint32_t numBytes, VkPipelineStageFlagBits dstStage, VkAccessFlags dstAccess)
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