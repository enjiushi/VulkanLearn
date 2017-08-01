#include "FrameManager.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Fence.h"
#include "GlobalDeviceObjects.h"
#include "SwapChain.h"
#include "PerFrameResource.h"
#include "Fence.h"
#include "Semaphore.h"
#include "CommandBuffer.h"
#include "Queue.h"
#include "../thread/ThreadTaskQueue.hpp"
#include <algorithm>
#include "Semaphore.h"
#include <stack>

bool FrameManager::Init(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount, const std::shared_ptr<FrameManager>& pSelf)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_currentPresentBin = 0;
	m_currentSemaphoreIndex = 0;
	m_maxFrameCount = maxFrameCount;

	for (uint32_t i = 0; i < maxFrameCount; i++)
	{
		m_frameResTable[i] = std::vector<std::shared_ptr<PerFrameResource>>();
		m_frameFences.push_back(Fence::Create(pDevice));
		m_threadTaskQueues.push_back(std::make_shared<ThreadTaskQueue>(pDevice, maxFrameCount, pSelf));
		m_acquireDoneSemaphores.push_back(Semaphore::Create(pDevice));
		m_renderDoneSemahpres.push_back(Semaphore::Create(pDevice));
	}

	m_jobStatus.resize(maxFrameCount);

	m_maxFrameCount = maxFrameCount;
	
	return true;
}

std::shared_ptr<FrameManager> FrameManager::Create(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount)
{
	std::shared_ptr<FrameManager> pFrameManager = std::make_shared<FrameManager>();
	if (pFrameManager.get() && pFrameManager->Init(pDevice, maxFrameCount, pFrameManager))
		return pFrameManager;
	return nullptr;
}

std::shared_ptr<PerFrameResource> FrameManager::AllocatePerFrameResource(uint32_t frameBinIndex)
{
	if (frameBinIndex < 0 || frameBinIndex >= m_maxFrameCount)
		return nullptr;

	std::shared_ptr<PerFrameResource> pPerFrameRes = PerFrameResource::Create(GetDevice(), frameBinIndex);
	m_frameResTable[frameBinIndex].push_back(pPerFrameRes);
	return pPerFrameRes;
}

void FrameManager::WaitForFence()
{
	WaitForFence(m_currentFrameIndex);
}

void FrameManager::WaitForFence(uint32_t frameIndex)
{
	m_frameFences[frameIndex]->Wait();
}

void FrameManager::WaitForAllJobsDone()
{
	for (uint32_t i = 0; i < m_threadTaskQueues.size(); i++)
	{
		m_threadTaskQueues[i]->WaitForFree();
		std::unique_lock<std::mutex> lock(m_mutex);
		WaitForGPUWork(i);
	}
}

// Increase bin index, frame manager moves to next bin, this function is called at the very beginning of a frame
// This function will make sure that current frame will perform correctly, since all previous dependency work has been finished
// This function will make sure that a lot of swapchain acquire will be available after calling this function
// The key is that if the count of previous cached work is more than (max frame count - 2), the oldest one needs to be flushed
// or swapchain acquire will fail
// 2 comes from min frame count of swapchain
void FrameManager::FlushIfNecessary()
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (m_frameIndexQueue.size() < m_maxFrameCount - 1)
		return;

	// Get oldest frame bin that maybe hasn't been flushed
	uint32_t oldestFrameIndex = m_frameIndexQueue.front();
	m_frameIndexQueue.pop_front();

	// Unlock and wait for workers complete their job for oldest bin
	// Workers visit frame manager once their work ends, so we have to unlock here or there could be inter-lock between frame manager and worker
	lock.unlock();

	// Wait for workers
	m_threadTaskQueues[oldestFrameIndex]->WaitForFree();

	// Workers completed their jobs, lock frame manager again
	lock.lock();

	// Wait for gpu work of oldest frame bin
	WaitForGPUWork(oldestFrameIndex);
}

void FrameManager::SetFrameIndex(uint32_t index)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_currentFrameIndex = index;
	m_frameIndexQueue.push_back(index);
}

void FrameManager::CacheSubmissioninfo(
	const std::shared_ptr<Queue>& pQueue,
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
	const std::vector<VkPipelineStageFlags>& waitStages,
	bool waitUtilQueueIdle)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	
	uint32_t frameIndex = cmdBuffer[0]->GetCommandPool()->GetPerFrameResource().lock()->GetFrameIndex();
	CacheSubmissioninfoInternal(pQueue, cmdBuffer, { m_acquireDoneSemaphores[m_jobStatus[frameIndex].semahporeIndex] }, waitStages, { m_renderDoneSemahpres[m_jobStatus[frameIndex].semahporeIndex] }, waitUtilQueueIdle);
}

void FrameManager::CacheSubmissioninfo(
	const std::shared_ptr<Queue>& pQueue,
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
	bool waitUtilQueueIdle)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	CacheSubmissioninfoInternal(pQueue, cmdBuffer, waitSemaphores, waitStages, signalSemaphores, waitUtilQueueIdle);
}

void FrameManager::CacheSubmissioninfoInternal(
	const std::shared_ptr<Queue>& pQueue,
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
	bool waitUtilQueueIdle)
{
#ifdef _DEBUG
	{
		ASSERTION(!cmdBuffer[0]->GetCommandPool()->GetPerFrameResource().expired());
		uint32_t frameIndex = cmdBuffer[0]->GetCommandPool()->GetPerFrameResource().lock()->GetFrameIndex();
		for (uint32_t i = 1; i < cmdBuffer.size(); i++)
		{
			ASSERTION(!cmdBuffer[i]->GetCommandPool()->GetPerFrameResource().expired());
			ASSERTION(frameIndex = cmdBuffer[i]->GetCommandPool()->GetPerFrameResource().lock()->GetFrameIndex());
		}
	}
#endif //_DEBUG
	uint32_t frameIndex = cmdBuffer[0]->GetCommandPool()->GetPerFrameResource().lock()->GetFrameIndex();

	SubmissionInfo info = 
	{
		pQueue,
		cmdBuffer,
		waitSemaphores,
		waitStages,
		signalSemaphores,
		waitUtilQueueIdle,
	};

	m_pendingSubmissionInfoTable[frameIndex].push_back(info);
}

void FrameManager::FlushCachedSubmission(uint32_t frameIndex)
{
	if (m_pendingSubmissionInfoTable[frameIndex].size() == 0)
		return;

	if (!m_jobStatus[frameIndex].submissionEnded || m_jobStatus[frameIndex].numJobs != 0)
		return;

	// Flush pending cmd buffers
	std::for_each(m_pendingSubmissionInfoTable[frameIndex].begin(), m_pendingSubmissionInfoTable[frameIndex].end(), [this, frameIndex](SubmissionInfo & info)
	{
		m_frameFences[frameIndex]->Reset();
		info.pQueue->SubmitCommandBuffers(info.cmdBuffers, info.waitSemaphores, info.waitStages, info.signalSemaphores, GetFrameFence(frameIndex), info.waitUtilQueueIdle);
	});

	// Add submitted cmd buffer references here, just to make sure they won't be deleted util this submission finished
	m_submissionInfoTable[frameIndex].insert(
		m_submissionInfoTable[frameIndex].end(),
		m_pendingSubmissionInfoTable[frameIndex].begin(),
		m_pendingSubmissionInfoTable[frameIndex].end());

	// Clear pending submissions
	m_pendingSubmissionInfoTable[frameIndex].clear();

	// First let know that this job is done and ready for present
	m_jobStatus[frameIndex].waitForPresent = true;

	m_jobStatus[frameIndex].callback(frameIndex, m_jobStatus[frameIndex].semahporeIndex);
	m_jobStatus[frameIndex].Reset();
	// FIXME: do this later

	/*
	// Can only start presentation when current presentation bin equals current frame bin
	// Do this to prevent those circumstances that frame 
	if (m_currentPresentBin == frameBinIndex)
	{
		// Starting from oldest bin index "(frameBinIndex + 1) % m_maxFrameCount", flush all pending presentation callbacks
		for (uint32_t i = m_currentPresentBin; i < m_maxFrameCount; i = (i + 1) % m_maxFrameCount)
		{
			// Flush cached present
			if (m_jobStatus[i].waitForPresent)
			{
				m_jobStatus[i].callback(FrameIndex(i));
				m_jobStatus[i].Reset();
			}
			// Not cached? stop here, wait until this bin's work finished
			else
			{
				m_currentPresentBin = i;
				break;
			}
		}
	}*/
}

// Add job to current frame bin
void FrameManager::AddJobToFrame(ThreadJobFunc jobFunc)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_threadTaskQueues[FrameIndex()]->AddJob(jobFunc, FrameIndex());
	m_jobStatus[FrameIndex()].numJobs++;
	m_jobStatus[FrameIndex()].semahporeIndex = m_currentSemaphoreIndex;
}

// Job done callback, will be invoked when worker threads finished their work
void FrameManager::JobDone(uint32_t frameIndex)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_jobStatus[frameIndex].numJobs--;
	FlushCachedSubmission(frameIndex);
}

// Wait until those gpu work of this frame finished
void FrameManager::WaitForGPUWork(uint32_t frameIndex)
{
	WaitForFence(frameIndex);
	m_submissionInfoTable[frameIndex].clear();
}

// End work submission, which means that current frame's work has been submitted completely
void FrameManager::EndJobSubmission(std::function<void(uint32_t, uint32_t)> callback)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_jobStatus[m_currentFrameIndex].callback = callback;
	m_jobStatus[m_currentFrameIndex].submissionEnded = true;

	// Try to flush cached submission if current frame work all has been done
	FlushCachedSubmission(m_currentFrameIndex);

	m_currentSemaphoreIndex = (m_currentSemaphoreIndex + 1) % m_maxFrameCount;
}

std::shared_ptr<Semaphore> FrameManager::GetAcqurieDoneSemaphore() const 
{
	return m_acquireDoneSemaphores[m_currentSemaphoreIndex]; 
}

std::shared_ptr<Semaphore> FrameManager::GetRenderDoneSemaphore() const 
{ 
	return m_renderDoneSemahpres[m_currentSemaphoreIndex];
}

std::shared_ptr<Semaphore> FrameManager::GetAcqurieDoneSemaphore(uint32_t index) const
{
	return m_acquireDoneSemaphores[index];
}

std::shared_ptr<Semaphore> FrameManager::GetRenderDoneSemaphore(uint32_t index) const
{
	return m_renderDoneSemahpres[index];
}