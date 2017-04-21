#include "ThreadCoordinator.h"

void ThreadCoordinator::AppendJob(std::function<void()> job)
{
	if (m_threadTaskQueue.GetTaskQueueSize() > 32)
	{
		m_threadTaskQueue.WaitForFree();
	}

	m_threadTaskQueue.AddJob(job);
}