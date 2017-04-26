#pragma once

#include "DeviceObjectBase.h"

template<typename T>
class DeviceObjSingleton : public DeviceObjectBase
{
public:
	static T* GetInstance()
	{
		if (m_instance)
			return m_instance;

		m_instance = new T;
		return m_instance;
	}

	static void Free()
	{
		if (m_instance)
			delete m_instance;
		m_instance = nullptr;
	}

public:
	bool Init(const std::shared_ptr<Device>& pDevice)
	{
		if (!DeviceObjectBase::Init(pDevice))
			return false;
		return true;
	}

protected:
	DeviceObjSingleton<T>() {}
	virtual ~DeviceObjSingleton<T>() {}

protected:
	static T* m_instance;
};

template <typename T>
T* DeviceObjSingleton<T>::m_instance = nullptr;