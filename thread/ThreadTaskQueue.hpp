#pragma once
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "../class/FrameWorkManager.h"
#include "ThreadWorker.hpp"

class Device;
class CommandBuffer;

class ThreadTaskQueue
{
public:
	ThreadTaskQueue(const std::shared_ptr<Device>& pDevice, uint32_t frameRoundBinCount, const std::shared_ptr<FrameWorkManager>& pFrameMgr)
	{
		m_worker = std::thread(&ThreadTaskQueue::Loop, this);

		int numThreads = std::thread::hardware_concurrency();
		//for (int i = 0; i < numThreads - 1; i++)
		// Only 1 thread worker for now
		for (int i = 0; i < 1; i++)
		{
			m_threadWorkers.push_back(std::make_shared<ThreadWorker>(pDevice, frameRoundBinCount, pFrameMgr));
		}

		m_currentWorker = 0;
	}

	~ThreadTaskQueue()
	{
		if (m_worker.joinable())
		{
			WaitForEmptyQueue();
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_isDestroying = true;
			lock.unlock();
			m_condition.notify_one();
			m_worker.join();
		}
	}

public:
	void AddJob(ThreadJobFunc jobFunc, uint32_t frameIndex)
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		ThreadWorker::ThreadJob job;
		job.job = jobFunc;
		job.frameIndex = frameIndex;
		job.pThreadTaskQueue = this;
		m_taskQueue.push(job);
		m_condition.notify_all();
	}

	void WaitForFree()
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_condition.wait(lock, [this]() { return m_taskQueue.empty() && !m_isSearchingThread; });
		for (uint32_t i = 0; i < m_threadWorkers.size(); i++)
		{
			m_threadWorkers[i]->WaitForFree();
		}
	}

	void WaitForEmptyQueue()
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_condition.wait(lock, [this]() { return m_taskQueue.empty() && !m_isSearchingThread; });
	}

	void WaitForWorkersAllFree()
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		for (uint32_t i = 0; i < m_threadWorkers.size(); i++)
		{
			m_threadWorkers[i]->WaitForFree();
		}
	}

	uint32_t GetTaskQueueSize()
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		return (uint32_t)m_taskQueue.size();
	}

private:
	void Loop()
	{
		while (true)
		{
			ThreadWorker::ThreadJob job;
			job.pThreadTaskQueue = this;
			{
				std::unique_lock<std::mutex> lock(m_queueMutex);
				m_condition.wait(lock, [this]() { return !m_taskQueue.empty() || m_isDestroying; });

				if (m_isDestroying)
				{
					break;
				}

				job = m_taskQueue.front();
				m_taskQueue.pop();

				m_isSearchingThread = true;

				m_condition.notify_all();
			}

			bool shouldExit = false;
			while (!shouldExit)
			{
				std::shared_ptr<ThreadWorker> pTWroker = m_threadWorkers[m_currentWorker];
				m_currentWorker = (m_currentWorker + 1) % m_threadWorkers.size();

				bool isFree = pTWroker->IsTaskQueueFree();
				if (isFree)
				{
					std::unique_lock<std::mutex> lock(m_queueMutex);

					//********************************************************************************************
					//** Important: I have to set states here before sending job to thread worker
					//** Or else, thread worker is busy, while states are still indicating current queue is free
					//** Then thread which is waiting for queue free will be fired, which is very very very wrong
					//** Damn this introduced a crash when frame rate is high, and this is really difficult to find
					//********************************************************************************************
					m_isSearchingThread = false;

					pTWroker->AppendJob(job);
					m_condition.notify_all();

					shouldExit = true;
				}
			}
		}
	}

private:
	std::mutex									m_queueMutex;
	std::thread									m_worker;
	std::condition_variable						m_condition;
	std::queue<ThreadWorker::ThreadJob>			m_taskQueue;
	std::vector<std::shared_ptr<ThreadWorker>>	m_threadWorkers;
	uint32_t									m_currentWorker;

	bool m_isSearchingThread =		false;
	bool m_isDestroying =			false;
	static bool m_isWorkerReady;
	uint32_t m_currentWorkerIndex = 0;
};