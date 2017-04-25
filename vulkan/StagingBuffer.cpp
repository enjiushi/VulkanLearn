#include "StagingBuffer.h"

bool StagingBuffer::Init(const std::shared_ptr<Device>& pDevice,
	uint32_t numBytes,
	const std::shared_ptr<DeviceMemoryManager>& pMemMgr,
	const void* pData)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	info.size = numBytes;
	if (!Buffer::Init(pDevice, info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, pMemMgr, pData))
		return false;

	return true;
}

std::shared_ptr<StagingBuffer> StagingBuffer::Create(const std::shared_ptr<Device>& pDevice,
	uint32_t numBytes,
	const std::shared_ptr<DeviceMemoryManager>& pMemMgr,
	const void* pData)
{
	std::shared_ptr<StagingBuffer> pStagingBuffer = std::make_shared<StagingBuffer>();
	if (pStagingBuffer.get() && pStagingBuffer->Init(pDevice, numBytes, pMemMgr, pData))
		return pStagingBuffer;
	return nullptr;
}