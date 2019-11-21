#pragma once

#include <memory>
#include <vector>

class Base
{
public:
	virtual ~Base() = 0 {}

	virtual bool Init() { return true; }

	void AddToReferenceTable(const std::shared_ptr<Base>& pObj) { m_referenceTable.push_back(pObj); }
	void SetName(const std::wstring& name) { m_name = name; m_nameHashCode = std::hash<std::wstring>()(m_name); }
	const std::wstring& GetName() const { return m_name; }
	std::size_t GetNameHashCode() const { return m_nameHashCode; }

protected:
	std::wstring						m_name;
	std::size_t							m_nameHashCode = 0;
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