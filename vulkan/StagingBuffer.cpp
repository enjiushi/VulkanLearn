#include "StagingBuffer.h"
#include "GlobalDeviceObjects.h"

bool StagingBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<StagingBuffer>& pSelf, uint32_t numBytes, VkAccessFlags accessFlags)
{
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	info.size = numBytes;
	if (!Buffer::Init(pDevice, pSelf, info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		return false;

	m_accessStages = VK_PIPELINE_STAGE_HOST_BIT;
	m_accessFlags = accessFlags;

	m_pData = DeviceMemMgr()->GetDataPtr(m_pMemKey, 0, numBytes);

	return true;
}

std::shared_ptr<StagingBuffer> StagingBuffer::Create(const std::shared_ptr<Device>& pDevice, uint32_t numBytes)
{
	std::shared_ptr<StagingBuffer> pStagingBuffer = std::make_shared<StagingBuffer>();
	if (pStagingBuffer.get() && pStagingBuffer->Init(pDevice, pStagingBuffer, numBytes, VK_ACCESS_HOST_WRITE_BIT))
		return pStagingBuffer;
	return nullptr;
}

std::shared_ptr<StagingBuffer> StagingBuffer::CreateReadableStagingBuffer(const std::shared_ptr<Device>& pDevice, uint32_t numBytes)
{
	std::shared_ptr<StagingBuffer> pStagingBuffer = std::make_shared<StagingBuffer>();
	if (pStagingBuffer.get() && pStagingBuffer->Init(pDevice, pStagingBuffer, numBytes, VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_HOST_READ_BIT))
		return pStagingBuffer;
	return nullptr;
}

void StagingBuffer::UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes)
{
	DeviceMemMgr()->UpdateBufferMemChunk(m_pMemKey, pData, offset, numBytes);
}