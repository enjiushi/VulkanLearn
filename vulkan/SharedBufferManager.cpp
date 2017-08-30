#include "SharedBufferManager.h"
#include "GlobalDeviceObjects.h"
#include "StagingBufferManager.h"

std::shared_ptr<BufferKey> BufferKey::Create(const std::shared_ptr<SharedBufferManager>& pSharedBufMgr, uint32_t index)
{
	std::shared_ptr<BufferKey> pBufKey = std::make_shared<BufferKey>();
	if (pBufKey.get() && pBufKey->Init(pSharedBufMgr, index))
		return pBufKey;
	return nullptr;
}

bool BufferKey::Init(const std::shared_ptr<SharedBufferManager>& pSharedBufMgr, uint32_t index)
{
	m_index = index;
	m_pSharedBufMgr = pSharedBufMgr;
	return true;
}

BufferKey::~BufferKey()
{
	m_pSharedBufMgr->FreeBuffer(m_index);
}

bool SharedBufferManager::Init(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<SharedBufferManager>& pSelf,
	VkBufferUsageFlags usage,
	uint32_t numBytes)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = usage;
	info.size = numBytes;
	m_pBuffer = Buffer::Create(pDevice, info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	return true;
}

std::shared_ptr<SharedBufferManager> SharedBufferManager::Create(const std::shared_ptr<Device>& pDevice,
	VkBufferUsageFlags usage,
	uint32_t numBytes)
{
	std::shared_ptr<SharedBufferManager> pSharedBufferManager = std::make_shared<SharedBufferManager>();
	if (pSharedBufferManager.get() && pSharedBufferManager->Init(pDevice, pSharedBufferManager, usage, numBytes))
		return pSharedBufferManager;
	return nullptr;
}

void SharedBufferManager::FreeBuffer(uint32_t index)
{
	m_bufferTable.erase(m_bufferTable.begin() + index);
}

std::shared_ptr<BufferKey> SharedBufferManager::AllocateBuffer(uint32_t numBytes)
{
	VkDescriptorBufferInfo info = {};
	info.buffer = GetBuffer()->GetDeviceHandle();
	uint32_t offset = 0;
	uint32_t endByte = 0;
	for (uint32_t i = 0; i < m_bufferTable.size(); i++)
	{
		endByte = offset + numBytes - 1;
		if (endByte < m_bufferTable[i].offset)
		{
			info.offset = offset;
			info.range = numBytes;
			m_bufferTable.insert(m_bufferTable.begin() + i, info);
			return BufferKey::Create(GetSelfSharedPtr(), i);
		}
		else
		{
			offset = m_bufferTable[i].offset + m_bufferTable[i].range;
		}
	}

	if (offset + numBytes > m_pBuffer->GetBufferInfo().size)
		return nullptr;

	info.offset = offset;
	info.range = numBytes;
	m_bufferTable.insert(m_bufferTable.end(), info);
	return BufferKey::Create(GetSelfSharedPtr(), m_bufferTable.size() - 1);
}

void SharedBufferManager::UpdateByteStream(const void* pData, const std::shared_ptr<Buffer>& pWrapperBuffer, const std::shared_ptr<BufferKey>& pBufKey, uint32_t offset, uint32_t numBytes)
{
	StagingBufferMgr()->UpdateByteStream(pWrapperBuffer, pData, offset + m_bufferTable[pBufKey->m_index].offset, numBytes);
}