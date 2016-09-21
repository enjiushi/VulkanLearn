#pragma once
#include "Base.h"
#include "AutoPTR.h"

class BaseObject;

class BaseComponent : public Base
{
public:
	BaseComponent(void);
	virtual ~BaseComponent(void) = 0;

	virtual void Update(GLfloat delta, GLboolean isDirtr = GL_FALSE) {}

	void SetObject(BaseObject* pObj) { m_object = pObj; }
	const BaseObject* GetObject() const { return m_object; }

protected:
	GLboolean m_isDirty;

	BaseObject* m_object;
};

