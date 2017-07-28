#pragma once
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>

class Device;
class ThreadTaskQueue;
class FrameManager;
class SwapChain;
class GlobalDeviceObjects;
class PerFrameResource;

typedef std::function<void(const std::shared_ptr<PerFrameResource>& pPerFrameRes)> ThreadJobFunc;

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

	ThreadWorker(const std::shared_ptr<Device>& pDevice, uint32_t frameRoundBinCount, const std::shared_ptr<FrameManager>& pFrameMgr);
	~ThreadWorker();

public:
	void AppendJob(ThreadJob job);
	bool IsTaskQueueFree();
	bool IsWorking();
	void Loop();
	void WaitForFree();

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