#pragma once
#include "Base.h"
#include "../common/Macros.h"
#include <mutex>

#define DECLARE_CLASS_RTTI(class_name)	\
public:	\
static std::size_t ClassHashCode;	\
virtual bool IsSameClass(std::size_t classHashCode) const override;	\

#define DEFINITE_CLASS_RTTI(class_name, base_class)	\
std::size_t	class_name::ClassHashCode = std::hash<std::string>()(TO_STRING(class_name));	\
bool class_name::IsSameClass(std::size_t classHashCode) const	\
{	\
	if (class_name::ClassHashCode == classHashCode)	\
		return true;	\
	return base_class::IsSameClass(classHashCode);	\
}

class BaseObject;
class PerFrameResource;

class BaseComponent : public SelfRefBase<BaseComponent>
{
public:
	static std::size_t	ClassHashCode;

public:
	virtual ~BaseComponent(void) {}

	virtual void Update() {}
	virtual void OnAnimationUpdate() {}
	virtual void LateUpdate() {}
	virtual void OnPreRender() {}
	virtual void CallbackFunc(std::shared_ptr<BaseObject>& pObject) {}
	virtual void OnRenderObject() {}
	virtual void OnPostRender() {}

	virtual void Awake() {}
	virtual void Start() {}

	std::shared_ptr<BaseObject> GetBaseObject() const 
	{
		if (!m_pObject.expired())
			return m_pObject.lock();
		return nullptr;
	}

	virtual bool IsSameClass(std::size_t classHashCode) const
	{
		return ClassHashCode == classHashCode;
	}

	template <typename T>
	std::shared_ptr<T> GetComponent(uint32_t index = 0) const
	{
		return GetBaseObject()->GetComponent<T>(index);
	}

	template <typename T>
	std::vector<std::shared_ptr<T>> GetComponents() const
	{
		return GetBaseObject()->GetComponents<T>();
	}

	template <typename T>
	bool DelComponent(uint32_t index = 0)
	{
		return GetBaseObject()->DelComponent<T>(index);
	}

	template <typename T>
	uint32_t DelComponents()
	{
		GetBaseObject()->DelComponents<T>();
	}

	template <typename T>
	bool ContainComponent(const std::shared_ptr<T>& pComp) const
	{
		return GetBaseObject()->ContainComponent<T>();
	}

protected:
	virtual bool Init(const std::shared_ptr<BaseComponent>& pSelf)
	{
		if (!SelfRefBase<BaseComponent>::Init(pSelf))
			return false;

		return true;
	}

	void SetObject(const std::shared_ptr<BaseObject>& pObject) { m_pObject = pObject; }

	// Will be called when it's been added to a base object
	void OnAddedToObject(const std::shared_ptr<BaseObject>& pObject) 
	{
		SetObject(pObject); 
		OnAddedToObjectInternal(pObject);
	}

	virtual void OnAddedToObjectInternal(const std::shared_ptr<BaseObject>& pObject) { }

protected:
	bool m_isDirty = false;

	std::weak_ptr<BaseObject>	m_pObject;
	std::mutex					m_updateMutex;

	friend class BaseObject;
};

