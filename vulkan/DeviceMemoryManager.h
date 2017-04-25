#pragma once
#include "DeviceObjectBase.h"
#include <map>

class DeviceMemoryManager : public DeviceObjectBase
{
	typedef struct _MemoryConsumeState
	{
		VkBuffer buffer;
		uint32_t startByte = 0;
		uint32_t endByte = 0;
	}MemoryConsumeState;

	typedef struct _MemoryNode
	{
		uint32_t numBytes = 0;
		VkDeviceMemory memory;
		std::vector<MemoryConsumeState> memoryConsumeState;
	}MemoryNode;

	static const uint32_t MEMORY_ALLOCATE_INC = 1024 * 1024;

public:
	~DeviceMemoryManager();

	bool Init(const std::shared_ptr<Device>& pDevice) override;
	void AllocateMem(VkBuffer buffer, uint32_t memoryPropertyBits, const void* pData = nullptr);

public:
	static std::shared_ptr<DeviceMemoryManager> Create(const std::shared_ptr<Device>& pDevice);

protected:
	void AllocateMemory(uint32_t numBytes, uint32_t memoryTypeBits, uint32_t memoryPropertyBits, VkDeviceMemory& memory, MemoryConsumeState& state);
	bool GetDeviceHandle(uint32_t typeIndex, VkDeviceMemory& deviceHandle);
	bool FindFreeMemorySegment(uint32_t typeIndex, uint32_t numBytes, MemoryConsumeState& state);
	void ReleaseMemory();

protected:
	std::map<uint32_t, MemoryNode> m_memoryPool;
};