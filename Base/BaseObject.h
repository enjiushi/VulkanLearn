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
	template <typename T>
	void AddComponent(const std::shared_ptr<T>& pComp)
	{
		if (ContainComponent(pComp))
			return;

		m_components.push_back(pComp);
		pComp->OnAddedToObject(GetSelfSharedPtr());
	}

	template <typename T>
	std::vector<std::shared_ptr<BaseComponent>>::const_iterator GetComponentIter(uint32_t index) const
	{
		uint32_t currentIndex = 0;
		auto iter = std::find_if(m_components.begin(), m_components.end(), [&currentIndex, index, classHashCode = T::ClassHashCode](auto & pComp)
		{
			return (currentIndex == index) && pComp->IsSameClass(classHashCode);
		});

		return iter;
	}

	template <typename T>
	std::shared_ptr<T> GetComponent(uint32_t index = 0) const
	{
		auto iter = GetComponentIter<T>(index);

		if (iter != m_components.end())
		{
			return std::static_pointer_cast<T>(*iter);
		}

		return nullptr;
	}

	template <typename T>
	std::vector<std::shared_ptr<T>> GetComponents() const
	{
		std::vector<std::shared_ptr<T>> resultVector;
		
		for (auto & pComp : m_components)
		{
			if (pComp->IsSameClass(T::ClassHashCode))
				resultVector.push_back(std::static_pointer_cast<T>(pComp));
		}

		return resultVector;
	}

	template <typename T>
	bool DelComponent(uint32_t index = 0)
	{
		auto iter = GetComponentIter<T>(index);

		if (iter != m_components.end())
		{
			m_components.erase(iter);
			return true;
		}

		return false;
	}

	template <typename T>
	uint32_t DelComponents()
	{
		uint32_t count = 0;
		bool removed;
		do
		{
			removed = false;

			auto iter = std::find_if(m_components.begin(), m_components.end(), [classHashCode = T::ClassHashCode](auto & pComp)
			{
				return pComp->IsSameClass(classHashCode);
			});

			if (iter != m_components.end())
			{
				m_components.erase(iter);
				removed = true;
				count++;
			}

		} while (removed);

		return count;
	}

	template <typename T>
	bool ContainComponent(const std::shared_ptr<T>& pComp) const
	{
		auto vec = GetComponents<T>();
		return std::find(vec.begin(), vec.end(), pComp) != vec.end();
	}

	void AddChild(const std::shared_ptr<BaseObject>& pObj);
	void DelChild(uint32_t index);
	std::shared_ptr<BaseObject> GetChild(uint32_t index);
	uint32_t GetChildrenCount() const { return (uint32_t)m_children.size(); }

	bool ContainObject(const std::shared_ptr<BaseObject>& pObj) const;

	void SetPos(const Vector3d& v) { m_localPosition = v; UpdateLocalTransform(); }
	void SetPos(double x, double y, double z) { m_localPosition = Vector3d(x, y, z); UpdateLocalTransform(); }
	void SetPosX(double x) { m_localPosition.x = x; UpdateLocalTransform(); }
	void SetPosY(double y) { m_localPosition.y = y; UpdateLocalTransform(); }
	void SetPosZ(double z) { m_localPosition.z = z; UpdateLocalTransform(); }

	void SetScale(const Vector3d& v) { m_localScale = v; UpdateLocalTransform(); }
	void SetScale(double x, double y, double z) { m_localScale = Vector3d(x, y, z); UpdateLocalTransform(); }
	void SetScaleX(double x) { m_localScale.x = x; UpdateLocalTransform(); }
	void SetScaleY(double y) { m_localScale.y = y; UpdateLocalTransform(); }
	void SetScaleZ(double z) { m_localScale.z = z; UpdateLocalTransform(); }

	void SetRotation(const Matrix3d& m);
	void SetRotation(const Quaterniond& q);

	void Rotate(const Vector3d& v, double angle);

public:
	void Update();
	void OnAnimationUpdate();
	void LateUpdate();
	void UpdateCachedData();
	void OnPreRender();
	void OnRenderObject();
	void OnPostRender();

	virtual void Awake();
	virtual void Start();

	Vector3d GetLocalPosition() const { return m_localPosition; }
	Vector3d GetWorldPosition() const;

	Matrix4d GetLocalTransform() const { return m_localTransform; }
	Matrix3d GetLocalRotationM() const { return m_localRotationM; }
	Quaterniond GetLocalRotationQ() const { return m_localRotationQ; }

	Matrix4d GetWorldTransform() const;
	Matrix3d GetWorldRotationM() const;
	Quaterniond GetWorldRotationQ() const;

	// These are before the stage of pre render
	Matrix4d GetCachedWorldTransform() const { return m_cachedWorldTransform; }
	Vector3d GetCachedWorldPosition() const { return m_cachedWorldPosition; }

	//creators
	static std::shared_ptr<BaseObject> Create();

protected:
	void UpdateLocalTransform();

protected:
	std::vector<std::shared_ptr<BaseComponent>>		m_components;
	std::vector<std::shared_ptr<BaseObject>>		m_children;
	std::weak_ptr<BaseObject>						m_pParent;

	Vector3d	m_localPosition;
	Vector3d	m_localScale;

	Matrix4d	m_localTransform;
	Matrix3d	m_localRotationM;
	Quaterniond m_localRotationQ;

	Matrix4d	m_cachedWorldTransform;
	Vector3d	m_cachedWorldPosition;
};