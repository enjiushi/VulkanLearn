#pragma once
#include "Base.h"

class BaseObject;

class BaseComponent : public Base
{
public:
	BaseComponent(void);
	virtual ~BaseComponent(void) = 0;

	virtual void Update(float delta, bool isDirtr = false) {}

	void SetObject(BaseObject* pObj) { m_object = pObj; }
	const BaseObject* GetObject() const { return m_object; }

protected:
	bool m_isDirty;

	BaseObject* m_object;
};

