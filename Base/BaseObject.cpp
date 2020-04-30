#include "BaseObject.h"
#include "../vulkan/GlobalDeviceObjects.h"

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
	Matrix4d cachedParentWorldTransform;

	if (!m_pParent.expired())
		cachedParentWorldTransform = m_pParent.lock()->m_cachedWorldTransform;

	m_cachedWorldTransform = cachedParentWorldTransform * m_localTransform;
	m_cachedWorldPosition = (cachedParentWorldTransform * Vector4d(m_localPosition, 1.0f)).xyz();

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
	//	FrameWorkMgr()->AddJobToFrame(std::bind(&BaseComponent::OnRenderObject, m_components[i].get(), std::placeholders::_1));
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

void BaseObject::SetRotation(const Matrix3d& m)
{
	m_localRotationM = m;
	m_localRotationQ = Quaterniond(m);
	UpdateLocalTransform();
}

void BaseObject::SetRotation(const Quaterniond& q)
{
	m_localRotationQ = q;
	m_localRotationM = q.Matrix();
	UpdateLocalTransform();
}

void BaseObject::UpdateLocalTransform()
{
	m_localTransform = Matrix4d(m_localRotationM * Matrix3d(Vector3d(m_localScale)), m_localPosition);
}

Vector3d BaseObject::GetWorldPosition() const
{
	Matrix4d parentWorldTransform;

	if (!m_pParent.expired())
		parentWorldTransform = m_pParent.lock()->GetWorldTransform();

	//get world transform
	return (parentWorldTransform * Vector4d(m_localPosition, 1.0)).xyz();
}

Matrix4d BaseObject::GetWorldTransform() const 
{ 
	Matrix4d parentWorldTransform;
	if (!m_pParent.expired())
		parentWorldTransform = m_pParent.lock()->GetWorldTransform();

	return parentWorldTransform * m_localTransform;
}

Matrix3d BaseObject::GetWorldRotationM() const
{ 
	Matrix3d parentWorldRotationM;
	if (!m_pParent.expired())
		parentWorldRotationM = m_pParent.lock()->GetWorldRotationM();

	return parentWorldRotationM * m_localRotationM;
}

Quaterniond BaseObject::GetWorldRotationQ() const
{ 
	return Quaterniond(GetWorldRotationM());
}

void BaseObject::Rotate(const Quaterniond& rotation)
{
	m_localRotationQ = rotation * m_localRotationQ;
	m_localRotationM = m_localRotationQ.Matrix();
	UpdateLocalTransform();
}

void BaseObject::Rotate(const Vector3d& v, double angle)
{
	Rotate({ v, angle });
}