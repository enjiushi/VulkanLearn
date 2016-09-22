#pragma once

template <typename T>
class Singleton
{
public:
	static T* GetInstance()
	{
		static T* instance = nullptr;
		if (instance)
			return instance;

		instance = new T;
		return instance;
	}

protected:
	Singleton<T>() {}
	~Singleton<T>() {}
};