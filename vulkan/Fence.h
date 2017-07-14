#pragma once

#include "DeviceObjectBase.h"

class Fence : public DeviceObjectBase<Fence>
{
public:
	~Fence();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Fence>& pSelf);

public:
	VkFence GetDeviceHandle() const { return m_fence; }
	bool Signaled() const { return m_signaled; }
	void Reset();
	void Wait() const;

public:
	static std::shared_ptr<Fence> Create(const std::shared_ptr<Device>& pDevice);

protected:
	VkFence	m_fence;
	bool	m_signaled;
};