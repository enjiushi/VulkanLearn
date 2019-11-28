#include "BaseObject.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/FrameManager.h"

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

	//update all children objects with global registered components
	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->Update();
}

void BaseObject::OnAnimationUpdate()
{
	for (size_t i = 0; i < m_components.size(); i++)
		m_components[i]->OnAnimationUpdate();

	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->OnAnimationUpdate();
}

void BaseObject::LateUpdate()
{
	for (size_t i = 0; i < m_components.size(); i++)
		m_components[i]->LateUpdate();

	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->LateUpdate();
}

void BaseObject::UpdateCachedData()
{
	Matrix4f cachedParentWorldTransform;

	if (!m_pParent.expired())
		cachedParentWorldTransform = m_pParent.lock()->m_cachedWorldTransform;

	m_cachedWorldTransform = cachedParentWorldTransform * m_localTransform;
	m_cachedWorldPosition = (cachedParentWorldTransform * Vector4f(m_localPosition, 1.0f)).xyz();

	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->UpdateCachedData();
}

void BaseObject::OnPreRender()
{
	for (size_t i = 0; i < m_components.size(); i++)
		m_components[i]->OnPreRender();

	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->OnPreRender();
}

void BaseObject::OnRenderObject()
{
	//update components attached to this object
	//for (size_t i = 0; i < m_components.size(); i++)
	//	FrameMgr()->AddJobToFrame(std::bind(&BaseComponent::OnRenderObject, m_components[i].get(), std::placeholders::_1));
	for (size_t i = 0; i < m_components.size(); i++)
		m_components[i]->OnRenderObject();

	//update all children objects
	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->OnRenderObject();
}

void BaseObject::OnPostRender()
{
	for (size_t i = 0; i < m_components.size(); i++)
		m_components[i]->OnPostRender();

	for (size_t i = 0; i < m_children.size(); i++)
		m_children[i]->OnPostRender();
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

	if (!m_pParent.expired())
		parentWorldTransform = m_pParent.lock()->GetWorldTransform();

	//get world transform
	return (parentWorldTransform * Vector4f(m_localPosition, 1.0f)).xyz();
}

Matrix4f BaseObject::GetWorldTransform() const 
{ 
	Matrix4f parentWorldTransform;
	if (!m_pParent.expired())
		parentWorldTransform = m_pParent.lock()->GetWorldTransform();

	return parentWorldTransform * m_localTransform;
}

Matrix3f BaseObject::GetWorldRotationM() const
{ 
	Matrix3f parentWorldRotationM;
	if (!m_pParent.expired())
		parentWorldRotationM = m_pParent.lock()->GetWorldRotationM();

	return parentWorldRotationM * m_localRotationM;
}

Quaternionf BaseObject::GetWorldRotationQ() const
{ 
	return Quaternionf(GetWorldRotationM());
}

void BaseObject::Rotate(const Vector3f& v, float angle)
{

}