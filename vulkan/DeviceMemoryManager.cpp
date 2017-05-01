#include "DeviceMemoryManager.h"
#include "MemoryConsumer.h"
#include "../common/Macros.h"
#include <algorithm>

DeviceMemoryManager::~DeviceMemoryManager()
{
	ReleaseMemory();
}

bool DeviceMemoryManager::Init(const std::shared_ptr<Device>& pDevice)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	return true;
}

std::shared_ptr<DeviceMemoryManager> DeviceMemoryManager::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<DeviceMemoryManager> pMgr = std::make_shared<DeviceMemoryManager>(DeviceMemoryManager());
	if (pMgr.get() && pMgr->Init(pDevice))
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

	pConsumer->BindMemory(m_memoryPool[typeIndex].memory, offset);

	m_bufferBindingTable[pConsumer].typeIndex = typeIndex;
	m_bufferBindingTable[pConsumer].startByte = offset;
	m_bufferBindingTable[pConsumer].numBytes = reqs.size;

	UpdateMemChunk(pConsumer, memoryPropertyBits, pData, offset, reqs.size);
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
	CHECK_VK_ERROR(vkMapMemory(GetDevice()->GetDeviceHandle(), m_memoryPool[bindingInfo.typeIndex].memory, bindingInfo.startByte + offset, updateNumBytes, 0, &pDeviceData));
	memcpy_s(pDeviceData, numBytes, pData, numBytes);
	vkUnmapMemory(GetDevice()->GetDeviceHandle(), m_memoryPool[m_bufferBindingTable[pConsumer].typeIndex].memory);
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

	if (m_memoryPool.find(typeIndex) == m_memoryPool.end())
	{
		MemoryNode node;
		node.numBytes = MEMORY_ALLOCATE_INC;

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = MEMORY_ALLOCATE_INC;
		allocInfo.memoryTypeIndex = typeIndex;
		CHECK_VK_ERROR(vkAllocateMemory(GetDevice()->GetDeviceHandle(), &allocInfo, nullptr, &node.memory));

		m_memoryPool[typeIndex] = node;
	}

	if (!FindFreeMemoryChunk(pMemConsumer, typeIndex, numBytes, offset))
	{ 
		// Should create a larger chunck of memory, do it later
		assert(false);
	}
}

void DeviceMemoryManager::FreeMemChunk(const MemoryConsumer* pConsumer)
{
	auto bindingIter = m_bufferBindingTable.find(pConsumer);
	if (bindingIter == m_bufferBindingTable.end())
		return;

	MemoryNode& node = m_memoryPool[bindingIter->second.typeIndex];
	auto memIter = std::find_if(node.bindingConsumerList.begin(), node.bindingConsumerList.end(), [pConsumer](const MemoryConsumer* pMemConsumer)
	{
		return pMemConsumer == pConsumer;
	});
	if (memIter == node.bindingConsumerList.end())
		return;

	m_bufferBindingTable.erase(pConsumer);
	node.bindingConsumerList.erase(memIter);
}

bool DeviceMemoryManager::FindFreeMemoryChunk(const MemoryConsumer* pMemConsumer, uint32_t typeIndex, uint32_t numBytes, uint32_t& offset)
{
 	offset = 0;
	uint32_t endByte = 0;
	for (uint32_t i = 0; i < m_memoryPool[typeIndex].bindingConsumerList.size(); i++)
	{
		BindingInfo bindingInfo = m_bufferBindingTable[m_memoryPool[typeIndex].bindingConsumerList[i]];

		endByte = offset + numBytes - 1;
		if (endByte < bindingInfo.startByte)
		{
			m_memoryPool[typeIndex].bindingConsumerList.insert(m_memoryPool[typeIndex].bindingConsumerList.begin() + i, pMemConsumer);
			return true;
		}
		else
		{
			offset = bindingInfo.startByte + bindingInfo.numBytes;
		}
	}

	if (offset + numBytes > m_memoryPool[typeIndex].numBytes)
		return false;

	m_memoryPool[typeIndex].bindingConsumerList.push_back(pMemConsumer);
	return true;
}

void DeviceMemoryManager::ReleaseMemory()
{
	std::for_each(m_memoryPool.begin(), m_memoryPool.end(), [this](std::pair<const uint32_t, MemoryNode>& pair)
	{
		vkFreeMemory(GetDevice()->GetDeviceHandle(), pair.second.memory, nullptr);
	});
}