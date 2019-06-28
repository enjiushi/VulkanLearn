#include "DeviceMemoryManager.h"
#include "MemoryConsumer.h"
#include "../common/Macros.h"
#include <algorithm>
#include "Buffer.h"
#include "Image.h"
#include "GlobalDeviceObjects.h"

uint32_t MemoryKey::m_allocatedKeys = 0;

std::shared_ptr<MemoryKey> MemoryKey::Create(bool bufferOrImage)
{
	std::shared_ptr<MemoryKey> pMemKey = std::make_shared<MemoryKey>();
	if (pMemKey.get() && pMemKey->Init(bufferOrImage))
		return pMemKey;
	return nullptr;
}

MemoryKey::~MemoryKey()
{
	if (m_bufferOrImage)
		DeviceMemMgr()->FreeBufferMemChunk(m_key);
	else
		DeviceMemMgr()->FreeImageMemChunk(m_key);
}

bool MemoryKey::Init(bool bufferOrImage)
{
	m_key = m_allocatedKeys++;
	m_bufferOrImage = bufferOrImage;
	return true;
}

DeviceMemoryManager::~DeviceMemoryManager()
{
	ReleaseMemory();
}

bool DeviceMemoryManager::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<DeviceMemoryManager>& pSelf)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_bufferMemPool.resize(sizeof(uint32_t) * 8, {});	// Same size as bit count of uint32_t, i.e. type index count

	return true;
}

std::shared_ptr<DeviceMemoryManager> DeviceMemoryManager::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<DeviceMemoryManager> pMgr = std::make_shared<DeviceMemoryManager>(DeviceMemoryManager());
	if (pMgr.get() && pMgr->Init(pDevice, pMgr))
		return pMgr;
	return nullptr;
}

std::shared_ptr<MemoryKey> DeviceMemoryManager::AllocateBufferMemChunk(const std::shared_ptr<Buffer>& pBuffer, uint32_t memoryPropertyBits, const void* pData)
{
	std::shared_ptr<MemoryKey> pMemKey = MemoryKey::Create(true);

	VkMemoryRequirements reqs = pBuffer->GetMemoryReqirments();

	uint32_t typeIndex;
	uint32_t offset;
	AllocateBufferMemory(pMemKey->m_key, reqs.size, reqs.memoryTypeBits, memoryPropertyBits, typeIndex, offset);

	// If key exceeds lookup table size, increase it
	if (pMemKey->m_key >= m_bufferBindingLookupTable.size())
	{
		std::vector<std::pair<uint32_t, bool>> newLookupTable(m_bufferBindingLookupTable.size() + LOOKUP_TABLE_SIZE_INC);
		std::swap_ranges(m_bufferBindingLookupTable.begin(), m_bufferBindingLookupTable.end(), newLookupTable.begin());
		m_bufferBindingLookupTable = std::move(newLookupTable);
	}

	pBuffer->BindMemory(m_bufferMemPool[typeIndex].memory, offset);

	uint32_t bindingTableIndex = m_bufferBindingTable.size();
	m_bufferBindingTable.push_back
		(
			{
				{
					typeIndex,
					offset,
					(uint32_t)reqs.size,
					nullptr
				},
				{
					false
				}
			}
		);

	if (m_bufferMemPool[typeIndex].pData)
		m_bufferBindingTable[bindingTableIndex].first.pData = (char*)m_bufferMemPool[typeIndex].pData + offset;

	m_bufferBindingLookupTable[pMemKey->m_key] = { bindingTableIndex, false };

	UpdateBufferMemChunk(pMemKey, memoryPropertyBits, pData, offset, reqs.size);

	return pMemKey;
}

std::shared_ptr<MemoryKey> DeviceMemoryManager::AllocateImageMemChunk(const std::shared_ptr<Image>& pImage, uint32_t memoryPropertyBits, const void* pData)
{
	std::shared_ptr<MemoryKey> pMemKey = MemoryKey::Create(false);

	VkMemoryRequirements reqs = pImage->GetMemoryReqirments();

	uint32_t typeIndex;
	uint32_t offset;
	AllocateImageMemory(pMemKey->m_key, reqs.size, reqs.memoryTypeBits, memoryPropertyBits, typeIndex, offset);
	pImage->BindMemory(m_imageMemPool[m_imageMemPoolLookupTable[pMemKey->m_key].first].memory, offset);

	return pMemKey;
}

bool DeviceMemoryManager::UpdateBufferMemChunk(const std::shared_ptr<MemoryKey>& pMemKey, uint32_t memoryPropertyBits, const void* pData, uint32_t offset, uint32_t numBytes)
{
	// Early return if it's been freed
	if (m_bufferBindingLookupTable[pMemKey->m_key].second)
		return false;

	if ((memoryPropertyBits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
		return false;

	if (pData == nullptr)
		return false;

	auto& bindingInfo = m_bufferBindingTable[m_bufferBindingLookupTable[pMemKey->m_key].first].first;

	// If numbytes is larger than buffer's bytes, use buffer bytes
	uint32_t updateNumBytes = numBytes > bindingInfo.numBytes ? bindingInfo.numBytes : numBytes;

	UpdateMemoryChunk(m_bufferMemPool[bindingInfo.typeIndex].memory, offset, updateNumBytes, bindingInfo.pData, pData);
	return true;
}

bool DeviceMemoryManager::UpdateImageMemChunk(const std::shared_ptr<MemoryKey>& pMemKey, uint32_t memoryPropertyBits, const void* pData, uint32_t offset, uint32_t numBytes)
{
	if ((memoryPropertyBits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
		return false;

	if (pData == nullptr)
		return false;

	auto& memoryNode = m_imageMemPool[m_imageMemPoolLookupTable[pMemKey->m_key].first];

	// If numbytes is larger than buffer's bytes, use buffer bytes
	uint32_t updateNumBytes = numBytes > memoryNode.numBytes ? memoryNode.numBytes : numBytes;

	UpdateMemoryChunk(memoryNode.memory, 0, updateNumBytes, memoryNode.pData, pData);
	return true;
}

void DeviceMemoryManager::UpdateMemoryChunk(VkDeviceMemory memory, uint32_t offset, uint32_t numBytes, void* pDst, const void* pData)
{
	memcpy_s((char*)pDst + offset, numBytes, pData, numBytes);
}

void DeviceMemoryManager::AllocateBufferMemory(uint32_t key, uint32_t numBytes, uint32_t memoryTypeBits, uint32_t memoryPropertyBits, uint32_t& typeIndex, uint32_t& offset)
{
	typeIndex = 0;
	uint32_t typeBits = memoryTypeBits;
	while (typeBits)
	{
		if (typeBits & 1)
		{
			int res = GetDevice()->GetPhysicalDevice()->GetPhysicalDeviceMemoryProperties().memoryTypes[typeIndex].propertyFlags & memoryPropertyBits;
			if (res == memoryPropertyBits)
			{
				break;
			}
		}
		typeBits >>= 1;
		typeIndex++;
	}

	if (m_bufferMemPool[typeIndex].memory == 0)
	{
		MemoryNode node;
		node.numBytes = (memoryPropertyBits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) ? STAGING_MEMORY_ALLOCATE_INC : DEVICE_MEMORY_ALLOCATE_INC;

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = node.numBytes;
		allocInfo.memoryTypeIndex = typeIndex;
		CHECK_VK_ERROR(vkAllocateMemory(GetDevice()->GetDeviceHandle(), &allocInfo, nullptr, &node.memory));

		if (memoryPropertyBits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			CHECK_VK_ERROR(vkMapMemory(GetDevice()->GetDeviceHandle(), node.memory, 0, numBytes, 0, &node.pData));

		m_bufferMemPool[typeIndex] = node;
	}

	if (!FindFreeBufferMemoryChunk(key, typeIndex, numBytes, offset))
	{ 
		// Should create a larger chunck of memory, do it later
		assert(false);
	}
}

void DeviceMemoryManager::AllocateImageMemory(uint32_t key, uint32_t numBytes, uint32_t memoryTypeBits, uint32_t memoryPropertyBits, uint32_t& typeIndex, uint32_t& offset)
{
	typeIndex = 0;
	uint32_t typeBits = memoryTypeBits;
	while (typeBits)
	{
		if (typeBits & 1)
		{
			if (GetDevice()->GetPhysicalDevice()->GetPhysicalDeviceMemoryProperties().memoryTypes[typeIndex].propertyFlags & memoryPropertyBits)
			{
				break;
			}
		}
		typeBits >>= 1;
		typeIndex++;
	}

	MemoryNode node;
	node.numBytes = numBytes;

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = numBytes;
	allocInfo.memoryTypeIndex = typeIndex;
	CHECK_VK_ERROR(vkAllocateMemory(GetDevice()->GetDeviceHandle(), &allocInfo, nullptr, &node.memory));

	offset = 0;

	uint32_t imageMemPoolIndex = m_imageMemPool.size();
	m_imageMemPool.push_back(node);

	if (key >= m_imageMemPoolLookupTable.size())
	{
		std::vector<std::pair<uint32_t, bool>> newLookupTable(m_imageMemPoolLookupTable.size() + LOOKUP_TABLE_SIZE_INC);
		std::swap_ranges(m_imageMemPoolLookupTable.begin(), m_imageMemPoolLookupTable.end(), newLookupTable.begin());
		m_imageMemPoolLookupTable = std::move(newLookupTable);
	}

	m_imageMemPoolLookupTable[key] = { imageMemPoolIndex, false };
}

void DeviceMemoryManager::FreeBufferMemChunk(uint32_t key)
{
	// Early return if it's freed
	if (m_bufferBindingLookupTable[key].second)
		return;

	auto& bindingInfo = m_bufferBindingTable[m_bufferBindingLookupTable[key].first];
	
	MemoryNode& node = m_bufferMemPool[bindingInfo.first.typeIndex];
	auto memIter = std::find_if(node.bindingList.begin(), node.bindingList.end(), [key](uint32_t poolKey)
	{
		return poolKey == key;
	});
	if (memIter == node.bindingList.end())
		return;

	m_bufferBindingLookupTable[key].second = true;
	m_bufferBindingTable[m_bufferBindingLookupTable[key].first].second = true;

	node.bindingList.erase(memIter);

	// Need to add some sort of logics to make buffer tables more compact
}

void DeviceMemoryManager::FreeImageMemChunk(uint32_t key)
{
	auto index = m_imageMemPoolLookupTable[key];

	vkFreeMemory(GetDevice()->GetDeviceHandle(), m_imageMemPool[index.first].memory, nullptr);

	m_imageMemPool.erase(m_imageMemPool.begin() + index.first);
	m_imageMemPoolLookupTable[key].second = true;

	// Need to add some sort of logics to make buffer tables more compact
}

bool DeviceMemoryManager::FindFreeBufferMemoryChunk(uint32_t key, uint32_t typeIndex, uint32_t numBytes, uint32_t& offset)
{
 	offset = 0;
	uint32_t endByte = 0;
	for (uint32_t i = 0; i < m_bufferMemPool[typeIndex].bindingList.size(); i++)
	{
		auto& bindingInfo = m_bufferBindingTable[m_bufferBindingLookupTable[m_bufferMemPool[typeIndex].bindingList[i]].first];

		// Skip if it's freed
		if (bindingInfo.second)
			continue;

		endByte = offset + numBytes - 1;
		if (endByte < bindingInfo.first.startByte)
		{
			m_bufferMemPool[typeIndex].bindingList.insert(m_bufferMemPool[typeIndex].bindingList.begin() + i, key);
			return true;
		}
		else
		{
			offset = bindingInfo.first.startByte + bindingInfo.first.numBytes;
		}
	}

	if (offset + numBytes > m_bufferMemPool[typeIndex].numBytes)
		return false;

	m_bufferMemPool[typeIndex].bindingList.push_back(key);
	return true;
}

void DeviceMemoryManager::ReleaseMemory()
{
	std::for_each(m_bufferMemPool.begin(), m_bufferMemPool.end(), [this](MemoryNode& node)
	{
		vkFreeMemory(GetDevice()->GetDeviceHandle(), node.memory, nullptr);
	});
}

void* DeviceMemoryManager::GetDataPtr(const std::shared_ptr<MemoryKey>& pMemKey, uint32_t offset, uint32_t numBytes)
{
	return m_bufferBindingTable[m_bufferBindingLookupTable[pMemKey->m_key].first].first.pData;
}