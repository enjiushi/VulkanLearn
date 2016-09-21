#pragma once
#include "../common/RefCounted.h"
#include <vector>
#include "../common/AutoPTR.h"
#include "BaseComponent.h"
#include "../Maths/Matrix.h"
#include "../Maths/Quaternion.h"

class BaseObject : public RefCounted
{
public:
	BaseObject() : m_parent(nullptr) {}
	~BaseObject();

public:
	void AddComponent(BaseComponent* pComp);
	void DelComponent(uint32_t index);
	BaseComponent* GetComponent(uint32_t index);

	void AddChild(BaseObject* pObj);
	void DelChild(uint32_t index);
	BaseObject* GetChild(uint32_t index);

	bool ContainComponent(const BaseComponent* pComp) const;
	bool ContainObject(const BaseObject* pObj) const;

	void SetPos(const Vector3f& v) { m_localPosition = v; m_isDirty = true; }
	void SetPos(float x, float y, float z) { m_localPosition = Vector3f(x, y, z); m_isDirty = true; }
	void SetPosX(float x) { m_localPosition.x = x; m_isDirty = true; }
	void SetPosY(float y) { m_localPosition.y = y; m_isDirty = true; }
	void SetPosZ(float z) { m_localPosition.z = z; m_isDirty = true; }

	void SetRotation(const Matrix3f& m);
	void SetRotation(const Quaternionf& q);

	void Rotate(const Vector3f& v, float angle);

	bool IsDirty() const { return m_isDirty; }

	virtual void Update(float delta, bool isParentDirty = false);

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

	bool		m_isDirty;
};