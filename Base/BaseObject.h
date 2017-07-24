#pragma once
#include <vector>
#include "BaseComponent.h"
#include "../maths/Matrix.h"
#include "../maths/Quaternion.h"

class BaseObject : public SelfRefBase<BaseObject>
{
public:
	void AddComponent(const std::shared_ptr<BaseComponent>& pComp);
	void DelComponent(uint32_t index);
	std::shared_ptr<BaseComponent> GetComponent(uint32_t index);

	void AddChild(const std::shared_ptr<BaseObject>& pObj);
	void DelChild(uint32_t index);
	std::shared_ptr<BaseObject> GetChild(uint32_t index);

	bool ContainComponent(const std::shared_ptr<BaseComponent>& pComp) const;
	bool ContainObject(const std::shared_ptr<BaseObject>& pObj) const;

	void SetPos(const Vector3f& v) { m_localPosition = v; m_isDirty = true; }
	void SetPos(float x, float y, float z) { m_localPosition = Vector3f(x, y, z); m_isDirty = true; }
	void SetPosX(float x) { m_localPosition.x = x; m_isDirty = true; }
	void SetPosY(float y) { m_localPosition.y = y; m_isDirty = true; }
	void SetPosZ(float z) { m_localPosition.z = z; m_isDirty = true; }

	void SetRotation(const Matrix3f& m);
	void SetRotation(const Quaternionf& q);

	void Rotate(const Vector3f& v, float angle);

	bool IsDirty() const { return m_isDirty; }

	virtual void Update(float delta);

	Vector3f GetLocalPosition() const { return m_localPosition; }
	Vector3f GetWorldPosition() const { return m_worldPosition; }

	Matrix4f GetLocalTransform() const { return m_localTransform; }
	Matrix3f GetLocalRotationM() const { return m_localRotationM; }
	Quaternionf GetLocalRotationQ() const { return m_localRotationQ; }

	Matrix4f GetWorldTransform() const { return m_worldTransform; }
	Matrix3f GetWorldRotationM() const { return m_worldRotationM; }
	Quaternionf GetWorldRotationQ() const { return m_worldRotationQ; }


	//creators
	std::shared_ptr<BaseObject> Create();
protected:
	void UpdateLocalInfo();
	void UpdateWorldInfo();

protected:
	std::vector<std::shared_ptr<BaseComponent>>		m_components;
	std::vector<std::shared_ptr<BaseObject>>		m_children;
	std::shared_ptr<BaseObject>						m_pParent;

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