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
	MemoryConsumeState state;
	AllocateMemory(reqs.size, reqs.memoryTypeBits, memoryPropertyBits, typeIndex, stateIndex, state);

	pConsumer->BindMemory(m_memoryPool[typeIndex].memory, state.startByte);

	m_bufferBindingTable[pConsumer].typeIndex = typeIndex;
	m_bufferBindingTable[pConsumer].comsumeStateIndex = stateIndex;

	UpdateMemChunk(pConsumer, memoryPropertyBits, pData, state.startByte, state.numBytes);
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
	MemoryNode node = m_memoryPool[bindingInfo.typeIndex];
	MemoryConsumeState state = node.memoryConsumeState[bindingInfo.comsumeStateIndex];

	// If numbytes is larger than buffer's bytes, use buffer bytes
	uint32_t updateNumBytes = numBytes > state.numBytes ? state.numBytes : numBytes;

	void* pDeviceData;
	CHECK_VK_ERROR(vkMapMemory(GetDevice()->GetDeviceHandle(), m_memoryPool[bindingInfo.typeIndex].memory, state.startByte + offset, updateNumBytes, 0, &pDeviceData));
	memcpy_s(pDeviceData, numBytes, pData, numBytes);
	vkUnmapMemory(GetDevice()->GetDeviceHandle(), m_memoryPool[m_bufferBindingTable[pConsumer].typeIndex].memory);
	return true;
}

void DeviceMemoryManager::AllocateMemory(uint32_t numBytes, uint32_t memoryTypeBits, uint32_t memoryPropertyBits, uint32_t& typeIndex, uint32_t& stateIndex, MemoryConsumeState& state)
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

	if (!FindFreeMemoryChunk(typeIndex, numBytes, stateIndex, state))
	{ 
		// Should create a larger chunck of memory, do it later
		assert(false);
	}
}

void DeviceMemoryManager::FreeMemChunk(const MemoryConsumer* pConsumer)
{
	auto resultIter = m_bufferBindingTable.find(pConsumer);
	if (resultIter == m_bufferBindingTable.end())
		return;

	auto toRemove = m_memoryPool[resultIter->second.typeIndex].memoryConsumeState.begin() + resultIter->second.comsumeStateIndex;
	m_memoryPool[resultIter->second.typeIndex].memoryConsumeState.erase(toRemove);
}

bool DeviceMemoryManager::FindFreeMemoryChunk(uint32_t typeIndex, uint32_t numBytes, uint32_t& stateIndex, MemoryConsumeState& state)
{
	auto& consumeState = m_memoryPool[typeIndex].memoryConsumeState;

 	uint32_t offset = 0;
	uint32_t endByte = 0;
	for (uint32_t i = 0; i < consumeState.size(); i++)
	{
		endByte = offset + numBytes - 1;
		if (endByte < consumeState[i].startByte)
		{
			state.startByte = offset;
			state.numBytes = numBytes;
			consumeState.insert(consumeState.begin() + i, state);
			stateIndex = i;
			return true;
		}
		else
		{
			offset = consumeState[i].startByte + consumeState[i].numBytes;
		}
	}

	if (offset + numBytes > m_memoryPool[typeIndex].numBytes)
		return false;

	state.startByte = offset;
	state.numBytes = numBytes;
	consumeState.push_back(state);
	stateIndex = consumeState.size() - 1;
	return true;
}

void DeviceMemoryManager::ReleaseMemory()
{
	std::for_each(m_memoryPool.begin(), m_memoryPool.end(), [this](std::pair<const uint32_t, MemoryNode>& pair)
	{
		vkFreeMemory(GetDevice()->GetDeviceHandle(), pair.second.memory, nullptr);
	});
}