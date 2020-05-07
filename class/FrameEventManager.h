#pragma once

#include "../common/Singleton.h"
#include "FrameEventListener.h"
#include <memory>
#include <unordered_set>

class FrameEventManager : public Singleton<FrameEventManager>
{
public:
	bool Init() { return true; }

public:
	// Maybe more
	void OnFrameBegin();
	void OnPostSceneTraversal();
	void OnPreCmdPreparation();
	void OnPreCmdSubmission();
	void OnFrameEnd();

public:
	void Register(const std::shared_ptr<IFrameEventListener>& pListener);
	void UnRegister(const std::shared_ptr<IFrameEventListener>& pListener);

private:
	std::unordered_set<std::shared_ptr<IFrameEventListener>>	m_Listeners;
};