#pragma once
#include <thread>
#include <functional>
#include "../common/Singleton.h"
#include "ThreadTaskQueue.hpp"

class CommandBuffer;

class ThreadCoordinator : public Singleton<ThreadCoordinator>
{
public:
	void AppendJob(ThreadJobFunc jobFunc, uint32_t frameIndex);

private:
	ThreadTaskQueue m_threadTaskQueue;
};