#pragma once
#include "Device.h"

template <class T>
class DeviceObjectBase
{
public:
	virtual ~DeviceObjectBase() {}

	virtual bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<T>& pSelf)
	{
		if (!pDevice.get())
			return false;

		m_pDevice = pDevice;
		m_pSelf = pSelf;
		return true;
	}

public:
	const std::shared_ptr<Device> GetDevice() const { return m_pDevice; }
	const std::shared_ptr<T> GetSelfSharedPtr() const { return m_pSelf.lock(); }

protected:
	std::shared_ptr<Device>	m_pDevice;
	std::weak_ptr<T>		m_pSelf;
};