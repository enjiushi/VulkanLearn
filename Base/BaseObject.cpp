#include "BaseObject.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/FrameManager.h"

std::vector<std::shared_ptr<BaseComponent>>	BaseObject::m_globalRegisteredComponents;

bool BaseObject::Init(const std::shared_ptr<BaseObject>& pObj)
{
	if (!SelfRefBase<BaseObject>::Init(pObj))
		return false;

	m_localScale = 1.0f;
	return true;
}

std::shared_ptr<BaseObject> BaseObject::Create()
{
	std::shared_ptr<BaseObject> pObj = std::make_shared<BaseObject>();
	if (pObj.get() && pObj->Init(pObj))
		return pObj;
	return nullptr;
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

bool BaseObject::ContainObject(const std::shared_ptr<BaseObject>& pObj) const
{
	for (size_t i = 0; i < m_children.size(); i++)
	{
		if (m_children[i] == pObj)
			return true;
	}
	return false;
}

void BaseObject::Update()
{
	//update components attached to this object
	for (size_t i = 0; i < m_components.size(); i++)
		m_components[i]->Update();

	// Call all global registered component call back function
	for (int32_t i = m_globalRegisteredComponents.size() - 1; i >= 0 && m_globalRegisteredComponents.size() > 0; i--)
		m_globalRegisteredComponents[i]->CallbackFunc(GetSelfSharedPtr());

	size_t originalSize = m_globalRegisteredComponents.size();

	// 1. Insert current base object's registered component into global vector
	if (m_localRegisteredComponents.size() != 0)
		m_globalRegisteredComponents.insert(m_globalRegisteredComponents.end(), m_localRegisteredComponents.begin(), m_localRegisteredComponents.end());

	//update all children objects with global registered components
	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->Update();

	// 2. Remove current base object's registered component from global vector
	if (m_localRegisteredComponents.size() != 0)
		m_globalRegisteredComponents.erase(m_globalRegisteredComponents.begin() + originalSize, m_globalRegisteredComponents.end());
}

void BaseObject::LateUpdate()
{
	//update components attached to this object
	for (size_t i = 0; i < m_components.size(); i++)
		m_components[i]->LateUpdate();

	//update all children objects
	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->LateUpdate();
}

void BaseObject::Draw()
{
	//update components attached to this object
	for (size_t i = 0; i < m_components.size(); i++)
		FrameMgr()->AddJobToFrame(std::bind(&BaseComponent::Draw, m_components[i].get(), std::placeholders::_1));

	//update all children objects
	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->Draw();
}

void BaseObject::Awake()
{
	std::for_each(m_components.begin(), m_components.end(), [](auto & pComp) { pComp->Awake(); });
	std::for_each(m_children.begin(), m_children.end(), [](auto & pChild) { pChild->Awake(); });
}

void BaseObject::Start()
{
	std::for_each(m_components.begin(), m_components.end(), [](auto & pComp) { pComp->Start(); });
	std::for_each(m_children.begin(), m_children.end(), [](auto & pChild) { pChild->Start(); });
}

void BaseObject::SetRotation(const Matrix3f& m)
{
	m_localRotationM = m;
	m_localRotationQ = Quaternionf(m);
	UpdateLocalTransform();
}

void BaseObject::SetRotation(const Quaternionf& q)
{
	m_localRotationQ = q;
	m_localRotationM = q.Matrix();
	UpdateLocalTransform();
}

void BaseObject::UpdateLocalTransform()
{
	m_localTransform = Matrix4f(m_localRotationM * Matrix3f(Vector3f(m_localScale)), m_localPosition);
}

Vector3f BaseObject::GetWorldPosition() const
{
	Matrix4f parentWorldTransform;
	if (m_pParent.get())
		parentWorldTransform = m_pParent->GetWorldTransform();

	//get world transform
	return (parentWorldTransform * Vector4f(m_localPosition, 1.0f)).xyz();
}

Matrix4f BaseObject::GetWorldTransform() const 
{ 
	Matrix4f parentWorldTransform;
	if (m_pParent.get())
		parentWorldTransform = m_pParent->GetWorldTransform();

	return parentWorldTransform * m_localTransform;
}

Matrix3f BaseObject::GetWorldRotationM() const
{ 
	Matrix3f parentWorldRotationM;
	if (m_pParent.get())
		parentWorldRotationM = m_pParent->GetWorldRotationM();

	return parentWorldRotationM * m_localRotationM;
}

Quaternionf BaseObject::GetWorldRotationQ() const
{ 
	return Quaternionf(GetWorldRotationM());
}

void BaseObject::Rotate(const Vector3f& v, float angle)
{

}

void BaseObject::RegisterCallbackComponent(const std::shared_ptr<BaseComponent>& pComponent)
{
	auto iter = std::find(m_localRegisteredComponents.begin(), m_localRegisteredComponents.end(), pComponent);
	if (iter != m_localRegisteredComponents.end())
		return;

	m_localRegisteredComponents.push_back(pComponent);
}
