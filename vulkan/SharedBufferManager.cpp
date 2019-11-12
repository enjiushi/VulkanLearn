#include "SharedBufferManager.h"
#include "GlobalDeviceObjects.h"
#include "StagingBufferManager.h"
#include "SharedBuffer.h"

uint32_t SharedBufferManager::m_numAllocatedKeys = 0;

std::shared_ptr<BufferKey> BufferKey::Create(const std::shared_ptr<SharedBufferManager>& pSharedBufMgr, uint32_t key)
{
	std::shared_ptr<BufferKey> pBufKey = std::make_shared<BufferKey>();
	if (pBufKey.get() && pBufKey->Init(pSharedBufMgr, key))
		return pBufKey;
	return nullptr;
}

bool BufferKey::Init(const std::shared_ptr<SharedBufferManager>& pSharedBufMgr, uint32_t key)
{
	m_key = key;
	m_pSharedBufMgr = pSharedBufMgr;
	return true;
}

BufferKey::~BufferKey()
{
	m_pSharedBufMgr->FreeBuffer(m_key);
}

bool SharedBufferManager::Init(const std::shared_ptr<Device>& pDevice,
	const std::shared_ptr<SharedBufferManager>& pSelf,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlagBits memFlag,
	uint32_t numBytes)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.usage = usage;
	info.size = numBytes;
	m_pBuffer = Buffer::Create(pDevice, info, memFlag);

	return true;
}

std::shared_ptr<SharedBufferManager> SharedBufferManager::Create(const std::shared_ptr<Device>& pDevice,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlagBits memFlag,
	uint32_t numBytes)
{
	std::shared_ptr<SharedBufferManager> pSharedBufferManager = std::make_shared<SharedBufferManager>();
	if (pSharedBufferManager.get() && pSharedBufferManager->Init(pDevice, pSharedBufferManager, usage, memFlag, numBytes))
		return pSharedBufferManager;
	return nullptr;
}

void SharedBufferManager::FreeBuffer(uint32_t index)
{
	uint32_t bufferChunkIndex = m_lookupTable[index];
	m_lookupTable.erase(index);
	m_bufferTable.erase(m_bufferTable.begin() + bufferChunkIndex);

	// Update lookup table since one record is removed, all records after it must be decreased
	for (auto& value : m_lookupTable)
	{
		if (value.second > bufferChunkIndex)
			value.second--;
	}
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
			// Update lookup table since one record is inserted, all records after it must be increased
			for (auto& value : m_lookupTable)
			{
				if (value.second >= i)
					value.second++;
			}

			// Insert buffer chunk
			info.offset = offset;
			info.range = numBytes;
			m_bufferTable.insert(m_bufferTable.begin() + i, info);

			// Update lookup table
			m_lookupTable[m_numAllocatedKeys] = i;

			// Generate BufferKey
			return BufferKey::Create(GetSelfSharedPtr(), m_numAllocatedKeys++);
		}
		else
		{
			offset = m_bufferTable[i].offset + m_bufferTable[i].range;
		}
	}

	if (offset + numBytes > m_pBuffer->GetBufferInfo().size)
		return nullptr;

	// Insert buffer chunk
	info.offset = offset;
	info.range = numBytes;
	m_bufferTable.insert(m_bufferTable.end(), info);

	// Update lookup table
	m_lookupTable[m_numAllocatedKeys] = m_bufferTable.size() - 1;

	// Generate BufferKey
	return BufferKey::Create(GetSelfSharedPtr(), m_numAllocatedKeys++);
}

void SharedBufferManager::UpdateByteStream(const void* pData, const std::shared_ptr<Buffer>& pWrapperBuffer, const std::shared_ptr<BufferKey>& pBufKey, uint32_t offset, uint32_t numBytes)
{
	if (m_pBuffer->IsHostVisible())
		m_pBuffer->UpdateByteStream(pData, offset + m_bufferTable[m_lookupTable[pBufKey->m_key]].offset, numBytes);
	// Since shared buffer manager holds a buffer shared by different shared buffers, with various usage and access flags, we can't simply let buffer do its update
	// Without specific buffer's information
	// So here we do a little hack to override, by directly call staging buffer to update wrapper buffer with its information
	else
		StagingBufferMgr()->UpdateByteStream(pWrapperBuffer, pData, offset + m_bufferTable[m_lookupTable[pBufKey->m_key]].offset, numBytes);
}

void SharedBufferManager::UpdateByteStream(const void* pData, const std::shared_ptr<SharedBuffer>& pWrapperBuffer, const std::shared_ptr<BufferKey>& pBufKey, uint32_t offset, uint32_t numBytes)
{
	if (m_pBuffer->IsHostVisible())
		m_pBuffer->UpdateByteStream(pData, offset + m_bufferTable[m_lookupTable[pBufKey->m_key]].offset, numBytes);
	else
		StagingBufferMgr()->UpdateByteStream(pWrapperBuffer, pData, offset + m_bufferTable[m_lookupTable[pBufKey->m_key]].offset, numBytes);
}

uint32_t SharedBufferManager::GetOffset(const std::shared_ptr<BufferKey>& pBufKey)
{ 
	return m_bufferTable[m_lookupTable[pBufKey->m_key]].offset;
}

VkDescriptorBufferInfo SharedBufferManager::GetBufferDesc(const std::shared_ptr<BufferKey>& pBufKey)
{ 
	return m_bufferTable[m_lookupTable[pBufKey->m_key]]; 
}