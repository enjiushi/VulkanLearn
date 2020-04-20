#pragma once

#include "DeviceObjectBase.h"

class Queue;

class Fence : public DeviceObjectBase<Fence>
{
public:
	enum FenceState
	{
		READ_FOR_USE,
		READ_FOR_SIGNAL,
		SIGNALED,
		COUNT
	};

public:
	~Fence();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Fence>& pSelf);

public:
	VkFence GetDeviceHandle() const { return m_fence; }
	FenceState GetFenceState() const { return m_fenceState; }
	bool Reset();
	bool Wait();

public:
	static std::shared_ptr<Fence> Create(const std::shared_ptr<Device>& pDevice);

private:
	VkFence		m_fence;
	FenceState	m_fenceState;

	friend class Queue;
};