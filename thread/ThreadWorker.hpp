#pragma once
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>

class Device;
class ThreadTaskQueue;
class FrameWorkManager;
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
	ThreadWorker(const std::shared_ptr<Device>& pDevice, uint32_t frameRoundBinCount, const std::shared_ptr<FrameWorkManager>& pFrameMgr);
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
};