#pragma once
#include "Base.h"

template <typename T>
class BaseSingleton : public Base
{
public:
	static T* GetInstance()
	{
		static T* instance = nullptr;
		if (instance)
			return instance;

		instance = new T;
		instance->Init();
		return instance;
	}

protected:
	BaseSingleton<T>() {}
	~BaseSingleton<T>() {}
};