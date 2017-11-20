#pragma once
#include "Base.h"

template <typename T>
class BaseSingleton : public SelfRefBase<T>
{
public:
	static std::shared_ptr<T>& GetInstance()
	{
		static std::shared_ptr<T> pInstance = nullptr;
		if (pInstance != nullptr)
			return pInstance;

		pInstance = std::make_shared<T>();
		pInstance->Init(pInstance);
		return pInstance;
	}

protected:
	BaseSingleton<T>() {}
	~BaseSingleton<T>() {}
};