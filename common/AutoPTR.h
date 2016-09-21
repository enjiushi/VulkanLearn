#pragma once

#include "RefCounted.h"

template <typename T>
class AutoPTR
{
public:
	AutoPTR() : m_ref(nullptr){}

	AutoPTR(const AutoPTR<T>& other)
	{
		m_ref = other.m_ref;
		if (m_ref)
			m_ref->AddRef();
	}

	AutoPTR(T* pointer) : m_ref(pointer)
	{
		if (m_ref)
			m_ref->AddRef();
	}

	AutoPTR(const T* pointer)
	{
		m_ref = const_cast<T*>(pointer);
		if (m_ref)
			m_ref->AddRef();
	}

	~AutoPTR() 
	{ 
		if (m_ref)
		{
			m_ref->DecRefNoDel();
			if (m_ref->GetRefCount() == 0)
			{
				delete m_ref;
				m_ref = nullptr;
			}
		}
	}

	T& operator*()
	{
		return *m_ref;
	}

	T* operator->()
	{
		return m_ref;
	}

	const T* operator->() const
	{
		return m_ref;
	}

	AutoPTR<T>& operator=(const T* rhs)
	{
		if (m_ref)
			m_ref->DecRef();
		m_ref = (T*)rhs;
		if (rhs != nullptr)
			m_ref->AddRef();
		return *this;
	}

	AutoPTR<T>& operator=(const AutoPTR<T>& rhs)
	{
		if (m_ref)
			m_ref->DecRef();
		m_ref = rhs.m_ref;
		if (rhs.m_ref != nullptr)
			m_ref->AddRef();
		return *this;
	}

	bool operator==(const T* rhs) const
	{
		return m_ref == rhs;
	}

	bool operator!=(const T* rhs) const
	{
		return m_ref != rhs;
	}

	operator T*() const
	{
		return m_ref;
	}

	T* Pointer()
	{
		return m_ref;
	}

private:
	T* m_ref;
};