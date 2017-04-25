#pragma once

#include "Buffer.h"

class UniformBuffer : public Buffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice,
		uint32_t numBytes,
		const std::shared_ptr<DeviceMemoryManager>& pMemMgr);

public:
	VkDescriptorBufferInfo GetDescBufferInfo() const 
	{
		VkDescriptorBufferInfo info;
		info.buffer = GetDeviceHandle();
		info.offset = 0;
		info.range = GetBufferInfo().size;
		return info;
	}

public:
	static std::shared_ptr<UniformBuffer> Create(const std::shared_ptr<Device>& pDevice,
		uint32_t numBytes,
		const std::shared_ptr<DeviceMemoryManager>& pMemMgr);
};