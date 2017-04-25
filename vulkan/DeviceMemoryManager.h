#pragma once
#include "DeviceObjectBase.h"

class DeviceMemoryManager : public DeviceObjectBase
{
public:
	~DeviceMemoryManager();

	bool Init(const std::shared_ptr<Device>& pDevice) override;

public:
	bool AllocateMemory(uint32_t byteSize);

public:
	static std::shared_ptr<DeviceMemoryManager> Create(const std::shared_ptr<Device>& pDevice);

protected:
	void ReleaseMemory();

protected:
	VkDeviceMemory	m_memory;
};