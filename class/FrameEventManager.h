#pragma once

#include "../common/Singleton.h"
#include "../Base/FrameEventListener.h"
#include <memory>
#include <unordered_set>

class FrameEventManager : public Singleton<FrameEventManager>
{
public:
	bool Init() {}

public:
	// Maybe more
	void OnFrameBegin();
	void OnFrameEnd();

public:
	void Register(const std::shared_ptr<IFrameEventListener>& pListener);
	void UnRegister(const std::shared_ptr<IFrameEventListener>& pListener);

private:
	std::unordered_set<std::shared_ptr<IFrameEventListener>>	m_Listeners;
};