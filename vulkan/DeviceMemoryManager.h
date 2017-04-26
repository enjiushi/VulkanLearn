#pragma once

#include "DeviceObjSingleton.h"
#include <map>

class Buffer;

class DeviceMemoryManager : public DeviceObjSingleton<DeviceMemoryManager>
{
	typedef struct _MemoryConsumeState
	{
		uint32_t startByte = 0;
		uint32_t numBytes = 0;
	}MemoryConsumeState;

	typedef struct _MemoryNode
	{
		uint32_t numBytes = 0;
		VkDeviceMemory memory;
		std::vector<MemoryConsumeState> memoryConsumeState;
	}MemoryNode;

	typedef struct _BufferBindingInfo
	{
		uint32_t typeIndex;
		uint32_t comsumeStateIndex;
	}BufferBindingInfo;

	static const uint32_t MEMORY_ALLOCATE_INC = 1024 * 1024 * 64;

public:
	~DeviceMemoryManager();

	bool Init(const std::shared_ptr<Device>& pDevice) override;

public:
	static std::shared_ptr<DeviceMemoryManager> Create(const std::shared_ptr<Device>& pDevice);

protected:
	void AllocateMemChunk(const Buffer* pBuffer, uint32_t memoryPropertyBits, const void* pData = nullptr);
	bool UpdateMemChunk(const Buffer* pBuffer, uint32_t memoryPropertyBits, const void* pData, uint32_t offset, uint32_t numBytes);
	void AllocateMemory(uint32_t numBytes, uint32_t memoryTypeBits, uint32_t memoryPropertyBits, uint32_t& typeIndex, uint32_t& stateIndex, MemoryConsumeState& state);
	bool FindFreeMemoryChunk(uint32_t typeIndex, uint32_t numBytes, uint32_t& stateIndex, MemoryConsumeState& state);
	void FreeMemChunk(const Buffer* pBuffer);
	void ReleaseMemory();

protected:
	std::map<uint32_t, MemoryNode>			m_memoryPool;
	std::map<const Buffer*, BufferBindingInfo>	m_bufferBindingTable;

	friend class Buffer;
};