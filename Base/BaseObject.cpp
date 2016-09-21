#include "stdafx.h"
#include "BaseObject.h"

BaseObject::~BaseObject()
{
}

void BaseObject::AddComponent(BaseComponent* pComp)
{
	if (ContainComponent(pComp))
		return;

	m_components.push_back(AutoPTR<BaseComponent>(pComp));
	pComp->SetObject(this);
}

void BaseObject::DelComponent(GLuint index)
{
	if (index < 0 || index >= m_components.size())
		return;
	m_components[index]->SetObject(nullptr);
	m_components.erase(m_components.begin() + index);
}

BaseComponent* BaseObject::GetComponent(GLuint index)
{
	if (index < 0 || index >= m_components.size())
		return nullptr;
	return m_components[index];
}

void BaseObject::AddChild(BaseObject* pObj)
{
	if (ContainObject(pObj))
		return;
	m_children.push_back(pObj);
	pObj->m_parent = this;
}

void BaseObject::DelChild(GLuint index)
{
	if (index < 0 || index >= m_children.size())
		return;
	m_children[index]->m_parent = nullptr;
	m_children.erase(m_children.begin() + index);
}

BaseObject* BaseObject::GetChild(GLuint index)
{
	if (index < 0 || index >= m_children.size())
		return nullptr;
	return m_children[index];
}

GLboolean BaseObject::ContainComponent(const BaseComponent* pComp) const
{
	for (size_t i = 0; i < m_components.size(); i++)
	{
		if (m_components[i] == pComp)
			return GL_TRUE;
	}
	return GL_FALSE;
}

GLboolean BaseObject::ContainObject(const BaseObject* pObj) const
{
	for (size_t i = 0; i < m_children.size(); i++)
	{
		if (m_children[i] == pObj)
			return GL_TRUE;
	}
	return GL_FALSE;
}

void BaseObject::Update(GLfloat delta, GLboolean isParentDirty)
{
	GLboolean isDirty = isParentDirty || m_isDirty;

	UpdateLocalInfo();

	//parent dirty or local dirty, all effect world transform
	if (isDirty)
		UpdateWorldInfo();

	//update components attached to this object
	for (size_t i = 0; i < m_components.size(); i++)
		m_components[i]->Update(delta, isDirty);

	//update all children objects
	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->Update(delta, isDirty);

	m_isDirty = GL_FALSE;
}

void BaseObject::SetRotation(const Matrix3f& m)
{
	m_localRotationM = m;
	m_localRotationQ = Quaternionf(m);
	m_isDirty = GL_TRUE;
}

void BaseObject::SetRotation(const Quaternionf& q)
{
	m_localRotationQ = q;
	m_localRotationM = q.Matrix();
	m_isDirty = GL_TRUE;
}

void BaseObject::UpdateLocalInfo()
{
	if (!m_isDirty)
		return;

	m_localTransform = Matrix4f(m_localRotationM, m_localPosition);
	m_localRotationQ = Quaternionf(m_localRotationM);
}

void BaseObject::UpdateWorldInfo()
{
	Matrix4f parentWorldTransform;
	Matrix3f parentWorldRotationM;
	if (m_parent)
	{
		parentWorldTransform = m_parent->m_worldTransform;
		parentWorldRotationM = m_parent->m_worldRotationM;
	}

	//get world transform
	m_worldTransform = parentWorldTransform * m_localTransform;
	m_worldRotationM = parentWorldRotationM * m_localRotationM;
	m_worldRotationQ = Quaternionf(m_worldRotationM);

	m_worldPosition = (parentWorldTransform * Vector4f(m_localPosition, 1.0f)).xyz();
}

void BaseObject::Rotate(const Vector3f& v, GLfloat angle)
{

}
