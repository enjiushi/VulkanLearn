#include "ThreadCoordinator.h"

void ThreadCoordinator::AppendJob(ThreadJobFunc jobFunc, uint32_t frameIndex)
{
	m_threadTaskQueue.AddJob(jobFunc, frameIndex);
}