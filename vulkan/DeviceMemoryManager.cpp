#include "DeviceMemoryManager.h"
#include "Buffer.h"
#include "../common/Macros.h"
#include <algorithm>

DeviceMemoryManager::~DeviceMemoryManager()
{
	ReleaseMemory();
}

bool DeviceMemoryManager::Init(const std::shared_ptr<Device>& pDevice)
{
	if (!DeviceObjSingleton<DeviceMemoryManager>::Init(pDevice))
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

void DeviceMemoryManager::AllocateMemChunk(const Buffer* pBuffer, uint32_t memoryPropertyBits, const void* pData)
{
	VkMemoryRequirements reqs;
	vkGetBufferMemoryRequirements(GetDevice()->GetDeviceHandle(), pBuffer->GetDeviceHandle(), &reqs);

	uint32_t typeIndex;
	uint32_t stateIndex;
	MemoryConsumeState state;
	AllocateMemory(reqs.size, reqs.memoryTypeBits, memoryPropertyBits, typeIndex, stateIndex, state);

	CHECK_VK_ERROR(vkBindBufferMemory(GetDevice()->GetDeviceHandle(), pBuffer->GetDeviceHandle(), m_memoryPool[typeIndex].memory, state.startByte));

	m_bufferBindingTable[pBuffer].typeIndex = typeIndex;
	m_bufferBindingTable[pBuffer].comsumeStateIndex = stateIndex;

	UpdateMemChunk(pBuffer, memoryPropertyBits, pData, state.startByte, state.numBytes);
}

bool DeviceMemoryManager::UpdateMemChunk(const Buffer* pBuffer, uint32_t memoryPropertyBits, const void* pData, uint32_t offset, uint32_t numBytes)
{
	if (m_bufferBindingTable.find(pBuffer) == m_bufferBindingTable.end())
		return false;

	if (memoryPropertyBits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT == 0)
		return false;

	if (pData == nullptr)
		return false;

	void* pDeviceData;
	CHECK_VK_ERROR(vkMapMemory(GetDevice()->GetDeviceHandle(), m_memoryPool[m_bufferBindingTable[pBuffer].typeIndex].memory, offset, numBytes, 0, &pDeviceData));
	memcpy_s(pDeviceData, numBytes, pData, numBytes);
	vkUnmapMemory(GetDevice()->GetDeviceHandle(), m_memoryPool[m_bufferBindingTable[pBuffer].typeIndex].memory);
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

void DeviceMemoryManager::FreeMemChunk(const Buffer* pBuffer)
{
	auto resultIter = m_bufferBindingTable.find(pBuffer);
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