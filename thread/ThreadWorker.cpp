#pragma once
#include "ThreadWorker.hpp"
#include "ThreadTaskQueue.hpp"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/SwapChain.h"
#include "../class/FrameWorkManager.h"

ThreadWorker::ThreadWorker(const std::shared_ptr<Device>& pDevice, uint32_t frameRoundBinCount) : m_isWorking(false)
{
	for (uint32_t i = 0; i < frameRoundBinCount; i++)
		m_frameRes.push_back(FrameWorkManager::GetInstance()->AllocatePerFrameResource(i));
	m_worker = std::thread(&ThreadWorker::Loop, this);
}

ThreadWorker::~ThreadWorker()
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

void ThreadWorker::Loop()
{
	while (true)
	{
		ThreadJob job;
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
		job.job(m_frameRes[job.frameIndex]);
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_isWorking = false;
			m_condition.notify_one();
		}
	}
}

void ThreadWorker::AppendJob(ThreadJob job)
{
	std::unique_lock<std::mutex> lock(m_queueMutex);
	m_jobQueue.push(job);
	m_condition.notify_one();
}

bool ThreadWorker::IsTaskQueueFree()
{
	bool ret = false;
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		ret = m_jobQueue.size() < m_jobQueueSize;
	}
	return ret;
}

bool ThreadWorker::IsWorking()
{
	bool ret = false;
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		ret = m_isWorking;
	}
	return ret;
}

void ThreadWorker::WaitForFree()
{
	std::unique_lock<std::mutex> lock(m_queueMutex);
	m_condition.wait(lock, [this] { return m_jobQueue.empty() && !m_isWorking; });
}
