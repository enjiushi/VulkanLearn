#pragma once

class RefCounted
{
public:
	RefCounted() : m_refCount(0) {}
	virtual ~RefCounted() = 0 {}

	void AddRef() { m_refCount++; }
	void DecRef() 
	{ 
		m_refCount--;
		if (m_refCount == 0)
			delete this;
	}
	void DecRefNoDel() { m_refCount--; }
	int GetRefCount() const { return m_refCount; }
	
private:
	int m_refCount;
};