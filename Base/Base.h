#pragma once

#include <memory>

class Base
{
public:
	Base() {}
	virtual ~Base() = 0 {}

	virtual bool Init() { return true; }
};

template <class T>
class SelfRefBase : public Base
{
public:
	const std::shared_ptr<T> GetSelfSharedPtr() const { return m_pSelf.lock(); }

protected:
	virtual bool Init(const std::shared_ptr<T>& pSelf)
	{
		if (!Base::Init())
			return false;

		m_pSelf = pSelf;
		return true;
	}

protected:
	std::weak_ptr<T> m_pSelf;
};