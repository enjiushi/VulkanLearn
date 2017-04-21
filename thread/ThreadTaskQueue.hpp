#pragma once
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "ThreadWorker.hpp"

class ThreadTaskQueue
{
public:
	ThreadTaskQueue()
	{
		m_worker = std::thread(&ThreadTaskQueue::Loop, this);
		//ThreadWorker worker;

		int numThreads = std::thread::hardware_concurrency();

		for (int i = 0; i < numThreads - 1; i++)
		{
			m_threadWorkers.push_back(std::make_shared<ThreadWorker>());
		}
	}

public:
	void AddJob(std::function<void()> job)
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_taskQueue.push(job);
		m_condition.notify_one();
	}

private:
	void Loop()
	{
		while (true)
		{
			std::function<void()> job;
			{
				std::unique_lock<std::mutex> lock(m_queueMutex);
				m_condition.wait(lock, [this]() { return !m_taskQueue.empty(); });
				job = m_taskQueue.front();
				m_taskQueue.pop();
			}

			bool shouldExit = false;
			while (!shouldExit)
			{
				std::find_if(m_threadWorkers.begin(), m_threadWorkers.end(), [&job, &shouldExit](std::shared_ptr<ThreadWorker>& worker)
				{
					bool isFree = worker->IsTaskQueueFree();
					if (isFree)
					{
						worker->AppendJob(job);
						shouldExit = true;
						return true;
					}
					else
						return false;

				});
			}
		}
	}

private:
	std::mutex m_queueMutex;
	std::thread m_worker;
	std::condition_variable m_condition;
	std::queue<std::function<void()>> m_taskQueue;
	std::vector<std::shared_ptr<ThreadWorker>> m_threadWorkers;
};