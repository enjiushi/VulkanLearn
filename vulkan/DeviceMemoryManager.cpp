#include "DeviceMemoryManager.h"
#include "MemoryConsumer.h"
#include "../common/Macros.h"
#include <algorithm>

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

void DeviceMemoryManager::AllocateMemChunk(const MemoryConsumer* pConsumer, uint32_t memoryPropertyBits, const void* pData)
{
	VkMemoryRequirements reqs = pConsumer->GetMemoryReqirments();

	uint32_t typeIndex;
	uint32_t stateIndex;
	uint32_t offset;
	AllocateMemory(pConsumer, reqs.size, reqs.memoryTypeBits, memoryPropertyBits, typeIndex, offset);

	if (pConsumer->BufferOrImage())
	{
		pConsumer->BindMemory(m_bufferMemPool[typeIndex].memory, offset);

		m_bufferBindingTable[pConsumer].typeIndex = typeIndex;
		m_bufferBindingTable[pConsumer].startByte = offset;
		m_bufferBindingTable[pConsumer].numBytes = reqs.size;

		UpdateMemChunk(pConsumer, memoryPropertyBits, pData, offset, reqs.size);
	}
	else
	{
		pConsumer->BindMemory(m_imageMemPool[pConsumer].memory, offset);
	}
}

bool DeviceMemoryManager::UpdateMemChunk(const MemoryConsumer* pConsumer, uint32_t memoryPropertyBits, const void* pData, uint32_t offset, uint32_t numBytes)
{
	if (m_bufferBindingTable.find(pConsumer) == m_bufferBindingTable.end())
		return false;

	if (memoryPropertyBits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT == 0)
		return false;

	if (pData == nullptr)
		return false;

	BindingInfo bindingInfo = m_bufferBindingTable[pConsumer];

	// If numbytes is larger than buffer's bytes, use buffer bytes
	uint32_t updateNumBytes = numBytes > bindingInfo.numBytes ? bindingInfo.numBytes : numBytes;

	void* pDeviceData;
	CHECK_VK_ERROR(vkMapMemory(GetDevice()->GetDeviceHandle(), m_bufferMemPool[bindingInfo.typeIndex].memory, bindingInfo.startByte + offset, updateNumBytes, 0, &pDeviceData));
	memcpy_s(pDeviceData, numBytes, pData, numBytes);
	vkUnmapMemory(GetDevice()->GetDeviceHandle(), m_bufferMemPool[m_bufferBindingTable[pConsumer].typeIndex].memory);
	return true;
}

void DeviceMemoryManager::AllocateMemory(const MemoryConsumer* pMemConsumer, uint32_t numBytes, uint32_t memoryTypeBits, uint32_t memoryPropertyBits, uint32_t& typeIndex, uint32_t& offset)
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

	if (!pMemConsumer->BufferOrImage())
	{
		MemoryNode node;
		node.numBytes = numBytes;

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = numBytes;
		allocInfo.memoryTypeIndex = typeIndex;
		CHECK_VK_ERROR(vkAllocateMemory(GetDevice()->GetDeviceHandle(), &allocInfo, nullptr, &node.memory));

		offset = 0;
		m_imageMemPool[pMemConsumer] = node;
		return;
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

	if (!FindFreeMemoryChunk(pMemConsumer, typeIndex, numBytes, offset))
	{ 
		// Should create a larger chunck of memory, do it later
		assert(false);
	}
}

void DeviceMemoryManager::FreeMemChunk(const MemoryConsumer* pConsumer)
{
	if (pConsumer->BufferOrImage())
	{
		auto bindingIter = m_bufferBindingTable.find(pConsumer);
		if (bindingIter == m_bufferBindingTable.end())
			return;

		MemoryNode& node = m_bufferMemPool[bindingIter->second.typeIndex];
		auto memIter = std::find_if(node.bindingConsumerList.begin(), node.bindingConsumerList.end(), [pConsumer](const MemoryConsumer* pMemConsumer)
		{
			return pMemConsumer == pConsumer;
		});
		if (memIter == node.bindingConsumerList.end())
			return;

		m_bufferBindingTable.erase(pConsumer);
		node.bindingConsumerList.erase(memIter);
	}
	else
	{
		auto imgIter = m_imageMemPool.find(pConsumer);
		if (imgIter == m_imageMemPool.end())
			return;

		vkFreeMemory(GetDevice()->GetDeviceHandle(), imgIter->second.memory, nullptr);
		m_imageMemPool.erase(imgIter);
	}
}

bool DeviceMemoryManager::FindFreeMemoryChunk(const MemoryConsumer* pMemConsumer, uint32_t typeIndex, uint32_t numBytes, uint32_t& offset)
{
 	offset = 0;
	uint32_t endByte = 0;
	for (uint32_t i = 0; i < m_bufferMemPool[typeIndex].bindingConsumerList.size(); i++)
	{
		BindingInfo bindingInfo = m_bufferBindingTable[m_bufferMemPool[typeIndex].bindingConsumerList[i]];

		endByte = offset + numBytes - 1;
		if (endByte < bindingInfo.startByte)
		{
			m_bufferMemPool[typeIndex].bindingConsumerList.insert(m_bufferMemPool[typeIndex].bindingConsumerList.begin() + i, pMemConsumer);
			return true;
		}
		else
		{
			offset = bindingInfo.startByte + bindingInfo.numBytes;
		}
	}

	if (offset + numBytes > m_bufferMemPool[typeIndex].numBytes)
		return false;

	m_bufferMemPool[typeIndex].bindingConsumerList.push_back(pMemConsumer);
	return true;
}

void DeviceMemoryManager::ReleaseMemory()
{
	std::for_each(m_bufferMemPool.begin(), m_bufferMemPool.end(), [this](std::pair<const uint32_t, MemoryNode>& pair)
	{
		vkFreeMemory(GetDevice()->GetDeviceHandle(), pair.second.memory, nullptr);
	});

	std::for_each(m_imageMemPool.begin(), m_imageMemPool.end(), [this](std::pair<const MemoryConsumer*, MemoryNode> pair)
	{
		vkFreeMemory(GetDevice()->GetDeviceHandle(), pair.second.memory, nullptr);
	});
}