#include "BaseObject.h"

std::shared_ptr<BaseObject> BaseObject::Create()
{
	std::shared_ptr<BaseObject> pObj = std::make_shared<BaseObject>();
	if (pObj.get() && pObj->Init(pObj))
		return pObj;
	return nullptr;
}

void BaseObject::AddComponent(const std::shared_ptr<BaseComponent>& pComp)
{
	if (ContainComponent(pComp))
		return;

	m_components.push_back(pComp);
	pComp->SetObject(GetSelfSharedPtr());
}

void BaseObject::DelComponent(uint32_t index)
{
	if (index < 0 || index >= m_components.size())
		return;

	m_components.erase(m_components.begin() + index);
}

std::shared_ptr<BaseComponent> BaseObject::GetComponent(uint32_t index)
{
	if (index < 0 || index >= m_components.size())
		return nullptr;
	return m_components[index];
}

void BaseObject::AddChild(const std::shared_ptr<BaseObject>& pObj)
{
	if (ContainObject(pObj))
		return;
	m_children.push_back(pObj);
	pObj->m_pParent = GetSelfSharedPtr();
}

void BaseObject::DelChild(uint32_t index)
{
	if (index < 0 || index >= m_children.size())
		return;
	m_children.erase(m_children.begin() + index);
}

std::shared_ptr<BaseObject> BaseObject::GetChild(uint32_t index)
{
	if (index < 0 || index >= m_children.size())
		return nullptr;
	return m_children[index];
}

bool BaseObject::ContainComponent(const std::shared_ptr<BaseComponent>& pComp) const
{
	return std::find(m_components.begin(), m_components.end(), pComp) != m_components.end();
}

bool BaseObject::ContainObject(const std::shared_ptr<BaseObject>& pObj) const
{
	for (size_t i = 0; i < m_children.size(); i++)
	{
		if (m_children[i] == pObj)
			return true;
	}
	return false;
}

void BaseObject::Update(float delta)
{
	UpdateLocalInfo();

	//parent dirty or local dirty, all effect world transform
	if (m_isDirty)
		UpdateWorldInfo();

	//update components attached to this object
	for (size_t i = 0; i < m_components.size(); i++)
		m_components[i]->Update(delta);

	//update all children objects
	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->Update(delta);

	m_isDirty = false;
}

void BaseObject::SetRotation(const Matrix3f& m)
{
	m_localRotationM = m;
	m_localRotationQ = Quaternionf(m);
	m_isDirty = true;
}

void BaseObject::SetRotation(const Quaternionf& q)
{
	m_localRotationQ = q;
	m_localRotationM = q.Matrix();
	m_isDirty = true;
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
	if (m_pParent.get())
	{
		parentWorldTransform = m_pParent->m_worldTransform;
		parentWorldRotationM = m_pParent->m_worldRotationM;
	}

	//get world transform
	m_worldTransform = parentWorldTransform * m_localTransform;
	m_worldRotationM = parentWorldRotationM * m_localRotationM;
	m_worldRotationQ = Quaternionf(m_worldRotationM);

	m_worldPosition = (parentWorldTransform * Vector4f(m_localPosition, 1.0f)).xyz();
}

void BaseObject::Rotate(const Vector3f& v, float angle)
{

}
