#pragma once
#include <vector>
#include "BaseComponent.h"
#include "../maths/Matrix.h"
#include "../maths/Quaternion.h"

class BaseObject : public SelfRefBase<BaseObject>
{
protected:
	bool Init(const std::shared_ptr<BaseObject>& pObj);

public:
	void AddComponent(const std::shared_ptr<BaseComponent>& pComp);
	void DelComponent(uint32_t index);
	std::shared_ptr<BaseComponent> GetComponent(uint32_t index);

	void AddChild(const std::shared_ptr<BaseObject>& pObj);
	void DelChild(uint32_t index);
	std::shared_ptr<BaseObject> GetChild(uint32_t index);

	bool ContainComponent(const std::shared_ptr<BaseComponent>& pComp) const;
	bool ContainObject(const std::shared_ptr<BaseObject>& pObj) const;

	void SetPos(const Vector3f& v) { m_localPosition = v; UpdateLocalTransform(); }
	void SetPos(float x, float y, float z) { m_localPosition = Vector3f(x, y, z); UpdateLocalTransform(); }
	void SetPosX(float x) { m_localPosition.x = x; UpdateLocalTransform(); }
	void SetPosY(float y) { m_localPosition.y = y; UpdateLocalTransform(); }
	void SetPosZ(float z) { m_localPosition.z = z; UpdateLocalTransform(); }

	void SetScale(const Vector3f& v) { m_localScale = v; UpdateLocalTransform(); }
	void SetScale(float x, float y, float z) { m_localScale = Vector3f(x, y, z); UpdateLocalTransform(); }
	void SetScaleX(float x) { m_localScale.x = x; UpdateLocalTransform(); }
	void SetScaleY(float y) { m_localScale.y = y; UpdateLocalTransform(); }
	void SetScaleZ(float z) { m_localScale.z = z; UpdateLocalTransform(); }

	void SetRotation(const Matrix3f& m);
	void SetRotation(const Quaternionf& q);

	void Rotate(const Vector3f& v, float angle);

	virtual void Update();
	virtual void LateUpdate();
	virtual void Draw();

	Vector3f GetLocalPosition() const { return m_localPosition; }
	Vector3f GetWorldPosition() const;

	Matrix4f GetLocalTransform() const { return m_localTransform; }
	Matrix3f GetLocalRotationM() const { return m_localRotationM; }
	Quaternionf GetLocalRotationQ() const { return m_localRotationQ; }

	Matrix4f BaseObject::GetWorldTransform() const;
	Matrix3f BaseObject::GetWorldRotationM() const;
	Quaternionf BaseObject::GetWorldRotationQ() const;

	//creators
	static std::shared_ptr<BaseObject> Create();
protected:
	void UpdateLocalTransform();

protected:
	std::vector<std::shared_ptr<BaseComponent>>		m_components;
	std::vector<std::shared_ptr<BaseObject>>		m_children;
	std::shared_ptr<BaseObject>						m_pParent;

	Vector3f	m_localPosition;
	Vector3f	m_localScale;

	Matrix4f	m_localTransform;
	Matrix3f	m_localRotationM;
	Quaternionf m_localRotationQ;
};