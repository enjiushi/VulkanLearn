#pragma once
#include "Base.h"

class BaseObject;
class PerFrameResource;

class BaseComponent : public SelfRefBase<BaseComponent>
{
public:
	virtual ~BaseComponent(void) {}

	virtual void Update(const std::shared_ptr<PerFrameResource>& pPerFrameRes) {}
	virtual void LateUpdate(const std::shared_ptr<PerFrameResource>& pPerFrameRes) {}

	std::shared_ptr<BaseObject> GetObject() const 
	{
		if (!m_pObject.expired())
			return m_pObject.lock();
		return nullptr;
	}

protected:
	virtual bool Init(const std::shared_ptr<BaseComponent>& pSelf)
	{
		if (!SelfRefBase<BaseComponent>::Init(pSelf))
			return false;

		return true;
	}

	void SetObject(const std::shared_ptr<BaseObject>& pObj) { m_pObject = pObj; }

protected:
	bool m_isDirty = false;

	std::weak_ptr<BaseObject> m_pObject;

	friend class BaseObject;
};

