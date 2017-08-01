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

	pBuffer->BindMemory(m_bufferMemPool[typeIndex].memory, offset);

	m_bufferBindingTable[pMemKey->m_key].typeIndex = typeIndex;
	m_bufferBindingTable[pMemKey->m_key].startByte = offset;
	m_bufferBindingTable[pMemKey->m_key].numBytes = reqs.size;

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
	pImage->BindMemory(m_imageMemPool[pMemKey->m_key].memory, offset);

	return pMemKey;
}

bool DeviceMemoryManager::UpdateBufferMemChunk(const std::shared_ptr<MemoryKey>& pMemKey, uint32_t memoryPropertyBits, const void* pData, uint32_t offset, uint32_t numBytes)
{
	if (m_bufferBindingTable.find(pMemKey->m_key) == m_bufferBindingTable.end())
		return false;

	if ((memoryPropertyBits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
		return false;

	if (pData == nullptr)
		return false;

	BindingInfo bindingInfo = m_bufferBindingTable[pMemKey->m_key];

	// If numbytes is larger than buffer's bytes, use buffer bytes
	uint32_t updateNumBytes = numBytes > bindingInfo.numBytes ? bindingInfo.numBytes : numBytes;

	UpdateMemoryChunk(m_bufferMemPool[bindingInfo.typeIndex].memory, bindingInfo.startByte + offset, updateNumBytes, pData);
	return true;
}

bool DeviceMemoryManager::UpdateImageMemChunk(const std::shared_ptr<MemoryKey>& pMemKey, uint32_t memoryPropertyBits, const void* pData, uint32_t offset, uint32_t numBytes)
{
	if (m_imageMemPool.find(pMemKey->m_key) == m_imageMemPool.end())
		return false;

	if ((memoryPropertyBits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
		return false;

	if (pData == nullptr)
		return false;

	MemoryNode memoryNode = m_imageMemPool[pMemKey->m_key];

	// If numbytes is larger than buffer's bytes, use buffer bytes
	uint32_t updateNumBytes = numBytes > memoryNode.numBytes ? memoryNode.numBytes : numBytes;

	UpdateMemoryChunk(memoryNode.memory, 0, updateNumBytes, pData);
	return true;
}

void DeviceMemoryManager::UpdateMemoryChunk(VkDeviceMemory memory, uint32_t offset, uint32_t numBytes, const void* pData)
{
	void* pDeviceData;
	CHECK_VK_ERROR(vkMapMemory(GetDevice()->GetDeviceHandle(), memory, offset, numBytes, 0, &pDeviceData));
	memcpy_s(pDeviceData, numBytes, pData, numBytes);
	vkUnmapMemory(GetDevice()->GetDeviceHandle(), memory);
}

void DeviceMemoryManager::AllocateBufferMemory(uint32_t key, uint32_t numBytes, uint32_t memoryTypeBits, uint32_t memoryPropertyBits, uint32_t& typeIndex, uint32_t& offset)
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

	if (m_bufferMemPool.find(typeIndex) == m_bufferMemPool.end())
	{
		MemoryNode node;
		node.numBytes = MEMORY_ALLOCATE_INC;

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = MEMORY_ALLOCATE_INC;
		allocInfo.memoryTypeIndex = typeIndex;
		CHECK_VK_ERROR(vkAllocateMemory(GetDevice()->GetDeviceHandle(), &allocInfo, nullptr, &node.memory));

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
	m_imageMemPool[key] = node;
}

void DeviceMemoryManager::FreeBufferMemChunk(uint32_t key)
{
	auto bindingIter = m_bufferBindingTable.find(key);
	if (bindingIter == m_bufferBindingTable.end())
		return;

	MemoryNode& node = m_bufferMemPool[bindingIter->second.typeIndex];
	auto memIter = std::find_if(node.bindingList.begin(), node.bindingList.end(), [key](uint32_t poolKey)
	{
		return poolKey == key;
	});
	if (memIter == node.bindingList.end())
		return;

	m_bufferBindingTable.erase(key);
	node.bindingList.erase(memIter);
}

void DeviceMemoryManager::FreeImageMemChunk(uint32_t key)
{
	auto memNodeIter = m_imageMemPool.find(key);
	if (memNodeIter == m_imageMemPool.end())
		return;

	m_imageMemPool.erase(key);
}

bool DeviceMemoryManager::FindFreeBufferMemoryChunk(uint32_t key, uint32_t typeIndex, uint32_t numBytes, uint32_t& offset)
{
 	offset = 0;
	uint32_t endByte = 0;
	for (uint32_t i = 0; i < m_bufferMemPool[typeIndex].bindingList.size(); i++)
	{
		BindingInfo bindingInfo = m_bufferBindingTable[m_bufferMemPool[typeIndex].bindingList[i]];

		endByte = offset + numBytes - 1;
		if (endByte < bindingInfo.startByte)
		{
			m_bufferMemPool[typeIndex].bindingList.insert(m_bufferMemPool[typeIndex].bindingList.begin() + i, key);
			return true;
		}
		else
		{
			offset = bindingInfo.startByte + bindingInfo.numBytes;
		}
	}

	if (offset + numBytes > m_bufferMemPool[typeIndex].numBytes)
		return false;

	m_bufferMemPool[typeIndex].bindingList.push_back(key);
	return true;
}

void DeviceMemoryManager::ReleaseMemory()
{
	std::for_each(m_bufferMemPool.begin(), m_bufferMemPool.end(), [this](std::pair<const uint32_t, MemoryNode>& pair)
	{
		vkFreeMemory(GetDevice()->GetDeviceHandle(), pair.second.memory, nullptr);
	});

	std::for_each(m_imageMemPool.begin(), m_imageMemPool.end(), [this](std::pair<const uint32_t, MemoryNode>& pair)
	{
		vkFreeMemory(GetDevice()->GetDeviceHandle(), pair.second.memory, nullptr);
	});
}