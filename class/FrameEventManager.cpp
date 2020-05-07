#include "FrameEventManager.h"

void FrameEventManager::OnFrameBegin()
{
	for each(auto pListener in m_Listeners)
	{
		pListener->OnFrameBegin();
	}
}

void FrameEventManager::OnPostSceneTraversal()
{
	for each(auto pListener in m_Listeners)
	{
		pListener->OnPostSceneTraversal();
	}
}

void FrameEventManager::OnPreCmdPreparation()
{
	for each(auto pListener in m_Listeners)
	{
		pListener->OnPreCmdPreparation();
	}
}

void FrameEventManager::OnPreCmdSubmission()
{
	for each(auto pListener in m_Listeners)
	{
		pListener->OnPreCmdSubmission();
	}
}

void FrameEventManager::OnFrameEnd()
{
	for each(auto pListener in m_Listeners)
	{
		pListener->OnFrameEnd();
	}
}

void FrameEventManager::Register(const std::shared_ptr<IFrameEventListener>& pListener)
{
	if (m_Listeners.find(pListener) != m_Listeners.end())
		return;

	m_Listeners.insert(pListener);
}

void FrameEventManager::UnRegister(const std::shared_ptr<IFrameEventListener>& pListener)
{
	m_Listeners.erase(pListener);
}