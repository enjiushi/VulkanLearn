#pragma once
#include "Base.h"
#include <mutex>

class BaseObject;
class PerFrameResource;

class BaseComponent : public SelfRefBase<BaseComponent>
{
public:
	virtual ~BaseComponent(void) {}

	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void Draw(const std::shared_ptr<PerFrameResource>& pPerFrameRes) {}

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

	std::weak_ptr<BaseObject>	m_pObject;
	std::mutex					m_updateMutex;

	friend class BaseObject;
};

