#include "StagingBuffer.h"
#include "GlobalDeviceObjects.h"

bool StagingBuffer::Init(const std::shared_ptr<Device>& pDevice,
	uint32_t numBytes,
	const void* pData)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	info.size = numBytes;
	if (!Buffer::Init(pDevice, info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, pData))
		return false;

	return true;
}

std::shared_ptr<StagingBuffer> StagingBuffer::Create(const std::shared_ptr<Device>& pDevice,
	uint32_t numBytes,
	const void* pData)
{
	std::shared_ptr<StagingBuffer> pStagingBuffer = std::make_shared<StagingBuffer>();
	if (pStagingBuffer.get() && pStagingBuffer->Init(pDevice, numBytes, pData))
		return pStagingBuffer;
	return nullptr;
}

void StagingBuffer::UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes)
{
	GlobalDeviceObjects::GetInstance()->GetDeviceMemMgr()->UpdateMemChunk(this, m_memProperty, pData, offset, numBytes);
}