#pragma once

#include "DeviceObjectBase.h"

class CommandPool;

class Semaphore : public DeviceObjectBase<Semaphore>
{
public:
	~Semaphore();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Semaphore>& pSelf);

public:
	VkSemaphore GetDeviceHandle() const { return m_semaphore; }

public:
	static std::shared_ptr<Semaphore> Create(const std::shared_ptr<Device>& pDevice);

protected:
	VkSemaphore	m_semaphore;
};