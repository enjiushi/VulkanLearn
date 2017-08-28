#pragma once

#include <memory>
#include <vector>

class Base
{
public:
	virtual ~Base() = 0 {}

	virtual bool Init() { return true; }

	void AddToReferenceTable(const std::shared_ptr<Base>& pObj) { m_referenceTable.push_back(pObj); }

protected:
	std::vector<std::shared_ptr<Base>>	m_referenceTable;
};

template <class T>
class SelfRefBase : public Base
{
public:
	std::shared_ptr<T> GetSelfSharedPtr() const 
	{
		if (!m_pSelf.expired())
			return m_pSelf.lock(); 
		return nullptr;
	}

	virtual ~SelfRefBase() {}

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