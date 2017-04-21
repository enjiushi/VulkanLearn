#pragma once
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>

class ThreadWorker
{
public:
	ThreadWorker() : m_isWorking(false)
	{
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
	void AppendJob(std::function<void()> job)
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

	void Loop()
	{
		while (true)
		{
			std::function<void()> job;
			{
				std::unique_lock<std::mutex> lock(m_queueMutex);
				m_condition.wait(lock, [this] { return !m_jobQueue.empty() || m_isDestroying; });

				if (m_isDestroying)
				{
					break;
				}

				job = m_jobQueue.front();
				m_jobQueue.pop();
				m_isWorking = true;
			}
			job();
			{
				std::unique_lock<std::mutex> lock(m_queueMutex);
				m_isWorking = false;
				m_condition.notify_one();
			}
		}
	}

	void WaitForFree()
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_condition.wait(lock, [this] { return m_jobQueue.empty() && !m_isWorking; });
	}

private:
	std::thread m_worker;
	std::mutex m_queueMutex;
	std::condition_variable m_condition;
	std::queue<std::function<void()>> m_jobQueue;
	bool m_isWorking = false;
	bool m_isDestroying = false;
	const int32_t m_jobQueueSize = 2;
};