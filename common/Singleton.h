#pragma once
#include <memory>

template <typename T>
class Singleton
{
public:
	static T* GetInstance()
	{
		if (m_pInstance != nullptr)
			return m_pInstance.get();

		m_pInstance = std::make_shared<T>();
		if (m_pInstance->Init())
			return m_pInstance.get();
		else
			return nullptr;
	}

	static void Free()
	{
	}

protected:
	Singleton<T>() {}
	virtual ~Singleton<T>() {}
	virtual bool Init() { return true; }

protected:
	static std::shared_ptr<T> m_pInstance;
};

template <typename T>
std::shared_ptr<T> Singleton<T>::m_pInstance = nullptr;