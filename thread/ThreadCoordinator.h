#pragma once
#include <thread>
#include <functional>
#include "../common/Singleton.h"
#include "ThreadTaskQueue.hpp"

class ThreadCoordinator : public Singleton<ThreadCoordinator>
{
public:
	void AppendJob(std::function<void()> job);

private:
	ThreadTaskQueue m_threadTaskQueue;
};