#pragma once

template <typename T>
class Singleton
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

protected:
	Singleton<T>() {}
	virtual ~Singleton<T>() {}

protected:
	static T* m_instance;
};

template <typename T>
T* Singleton<T>::m_instance = nullptr;