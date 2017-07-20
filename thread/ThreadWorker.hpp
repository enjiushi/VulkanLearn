#pragma once
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "ThreadCoordinator.h"
#include "../vulkan/PerFrameResource.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/FrameManager.h"

class Device;
class ThreadTaskQueue;

class ThreadWorker
{
public:
	typedef struct _ThreadJob
	{
		ThreadJobFunc	job;
		uint32_t		frameIndex;
		ThreadTaskQueue* pThreadTaskQueue = nullptr;
	}ThreadJob;

public:
	static void InitThreadWorkers(const std::shared_ptr<Device>& pDevice, uint32_t frameRoundBinCount, const std::shared_ptr<FrameManager>& pFrameMgr)
	{
		int numThreads = std::thread::hardware_concurrency();
		for (int i = 0; i < numThreads - frameRoundBinCount; i++)
		{
			m_threadWorkers.push_back(std::make_shared<ThreadWorker>(pDevice, frameRoundBinCount, pFrameMgr));
		}
	}

	static void UninitThreadWorkers()
	{
		m_threadWorkers.clear();
	}

	static std::shared_ptr<ThreadWorker> GetCurrentThreadWorker()
	{
		uint32_t temp = m_currentThreadWorker;
		m_currentThreadWorker = (m_currentThreadWorker + 1) % m_threadWorkers.size();
		return m_threadWorkers[temp];
	}

	ThreadWorker(const std::shared_ptr<Device>& pDevice, uint32_t frameRoundBinCount, const std::shared_ptr<FrameManager>& pFrameMgr) : m_isWorking(false)
	{
		for (uint32_t i = 0; i < frameRoundBinCount; i++)
			m_frameRes.push_back(pFrameMgr->AllocatePerFrameResource(i));
		m_worker = std::thread(&ThreadWorker::Loop, this);
	}

	~ThreadWorker()
	{
		if (m_worker.joinable())
		{
			WaitForFree();
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_isDestroying = true;
			lock.unlock();
			m_condition.notify_one();
			m_worker.join();
		}
	}

public:
	void AppendJob(ThreadJob job)
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_jobQueue.push(job);
		m_condition.notify_one();
	}

	bool IsTaskQueueFree()
	{
		bool ret = false;
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			ret = m_jobQueue.size() < m_jobQueueSize;
		}
		return ret;
	}
	
	bool IsWorking()
	{
		bool ret = false;
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			ret = m_isWorking;
		}
		return ret;
	}

	void Loop();

	void WaitForFree()
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_condition.wait(lock, [this] { return m_jobQueue.empty() && !m_isWorking; });
	}

private:
	std::thread					m_worker;
	std::mutex					m_queueMutex;
	std::condition_variable		m_condition;
	std::queue<ThreadJob>		m_jobQueue;
	std::vector<std::shared_ptr<PerFrameResource>>	m_frameRes;

	bool m_isWorking = false;
	bool m_isDestroying = false;
	const int32_t m_jobQueueSize = 2;

	static std::vector<std::shared_ptr<ThreadWorker>>	m_threadWorkers;
	static uint32_t										m_currentThreadWorker;
};