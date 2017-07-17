#pragma once

#include "DeviceObjectBase.h"

class Queue;

class Fence : public DeviceObjectBase<Fence>
{
public:
	~Fence();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Fence>& pSelf);

public:
	VkFence GetDeviceHandle() const { return m_fence; }
	bool Signaled() const { return m_signaled; }
	bool Submitted() const { return m_submitted; }
	void Reset();
	void Wait();

public:
	static std::shared_ptr<Fence> Create(const std::shared_ptr<Device>& pDevice);

protected:
	void SetSubmitted(bool flag) { m_submitted = flag; }

protected:
	VkFence	m_fence;
	bool	m_signaled;
	bool	m_submitted;

	friend class Queue;
};