#pragma once

#include "DeviceObjectBase.h"

class Queue : public DeviceObjectBase
{
public:
	~Queue();

	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t queueIndex);

public:
	VkQueue GetDeviceHandle() { return m_queue; }

public:
	static std::shared_ptr<Queue> Create(const std::shared_ptr<Device>& pDevice, uint32_t queueIndex);

protected:
	VkQueue m_queue;
};