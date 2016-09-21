#pragma once
#include "RefCounted.h"
#include <GL\glew.h>
#include <vector>
#include "AutoPTR.h"
#include "BaseComponent.h"
#include "Matrix.h"
#include "Quaternion.h"

class BaseObject : public RefCounted
{
public:
	BaseObject() : m_parent(nullptr) {}
	~BaseObject();

public:
	void AddComponent(BaseComponent* pComp);
	void DelComponent(GLuint index);
	BaseComponent* GetComponent(GLuint index);

	void AddChild(BaseObject* pObj);
	void DelChild(GLuint index);
	BaseObject* GetChild(GLuint index);

	GLboolean ContainComponent(const BaseComponent* pComp) const;
	GLboolean ContainObject(const BaseObject* pObj) const;

	void SetPos(const Vector3f& v) { m_localPosition = v; m_isDirty = GL_TRUE; }
	void SetPos(GLfloat x, GLfloat y, GLfloat z) { m_localPosition = Vector3f(x, y, z); m_isDirty = GL_TRUE; }
	void SetPosX(GLfloat x) { m_localPosition.x = x; m_isDirty = GL_TRUE; }
	void SetPosY(GLfloat y) { m_localPosition.y = y; m_isDirty = GL_TRUE; }
	void SetPosZ(GLfloat z) { m_localPosition.z = z; m_isDirty = GL_TRUE; }

	void SetRotation(const Matrix3f& m);
	void SetRotation(const Quaternionf& q);

	void Rotate(const Vector3f& v, GLfloat angle);

	GLboolean IsDirty() const { return m_isDirty; }

	virtual void Update(GLfloat delta, GLboolean isParentDirty = GL_FALSE);

	Vector3f GetLocalPosition() const { return m_localPosition; }
	Vector3f GetWorldPosition() const { return m_worldPosition; }

	Matrix4f GetLocalTransform() const { return m_localTransform; }
	Matrix3f GetLocalRotationM() const { return m_localRotationM; }
	Quaternionf GetLocalRotationQ() const { return m_localRotationQ; }

	Matrix4f GetWorldTransform() const { return m_worldTransform; }
	Matrix3f GetWorldRotationM() const { return m_worldRotationM; }
	Quaternionf GetWorldRotationQ() const { return m_worldRotationQ; }


	//creators
	BaseObject* Create() { return new BaseObject; }
protected:
	void UpdateLocalInfo();
	void UpdateWorldInfo();

protected:
	std::vector<AutoPTR<BaseComponent> > m_components;
	std::vector<AutoPTR<BaseObject> >	 m_children;
	const BaseObject*					 m_parent;

	Vector3f	m_localPosition;
	Vector3f	m_worldPosition;

	Matrix4f	m_localTransform;
	Matrix3f	m_localRotationM;
	Quaternionf m_localRotationQ;

	Matrix4f	m_worldTransform;
	Matrix3f	m_worldRotationM;
	Quaternionf m_worldRotationQ;

	GLboolean	m_isDirty;
};