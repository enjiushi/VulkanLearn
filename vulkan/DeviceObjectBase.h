#pragma once
#include "Device.h"

class DeviceObjectBase
{
public:
	virtual ~DeviceObjectBase() {}

	virtual bool Init(const std::shared_ptr<Device>& pDevice)
	{
		if (!pDevice.get())
			return false;

		m_pDevice = pDevice;
		return true;
	}

public:
	const std::shared_ptr<Device> GetDevice() const { return m_pDevice; }

protected:
	std::shared_ptr<Device> m_pDevice;
};