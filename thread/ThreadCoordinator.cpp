#include "ThreadCoordinator.h"

void ThreadCoordinator::AppendJob(std::function<void()> job)
{
	m_threadTaskQueue.AddJob(job);
}