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
	uint32_t GetChildrenCount() const { return m_children.size(); }

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

	void Update();
	void OnAnimationUpdate();
	void LateUpdate();
	void OnPreRender();
	void OnRenderObject();
	void OnPostRender();

	virtual void Awake();
	virtual void Start();

	Vector3f GetLocalPosition() const { return m_localPosition; }
	Vector3f GetWorldPosition() const;

	Matrix4f GetLocalTransform() const { return m_localTransform; }
	Matrix3f GetLocalRotationM() const { return m_localRotationM; }
	Quaternionf GetLocalRotationQ() const { return m_localRotationQ; }

	Matrix4f GetWorldTransform() const;
	Matrix3f GetWorldRotationM() const;
	Quaternionf GetWorldRotationQ() const;

	// These are before the stage of pre render
	Matrix4f GetCachedWorldTransform() const { return m_cachedWorldTransform; }
	Vector3f GetCachedWorldPosition() const { return m_cachedWorldPosition; }

	//creators
	static std::shared_ptr<BaseObject> Create();

protected:
	void UpdateLocalTransform();

protected:
	std::vector<std::shared_ptr<BaseComponent>>		m_components;
	std::vector<std::shared_ptr<BaseObject>>		m_children;
	std::weak_ptr<BaseObject>						m_pParent;

	Vector3f	m_localPosition;
	Vector3f	m_localScale;

	Matrix4f	m_localTransform;
	Matrix3f	m_localRotationM;
	Quaternionf m_localRotationQ;

	Matrix4f	m_cachedWorldTransform;
	Vector3f	m_cachedWorldPosition;
};