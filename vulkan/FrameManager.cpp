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

	m_currentSemaphoreIndex = 0;
	m_maxFrameCount = maxFrameCount;

	m_pThreadTaskQueue = std::make_shared<ThreadTaskQueue>(pDevice, maxFrameCount, pSelf);

	for (uint32_t i = 0; i < maxFrameCount; i++)
	{
		m_frameResTable[i] = std::vector<std::shared_ptr<PerFrameResource>>();
		m_frameFences.push_back(Fence::Create(pDevice));
		//m_threadTaskQueues.push_back(std::make_shared<ThreadTaskQueue>(pDevice, maxFrameCount, pSelf));
		m_acquireDoneSemaphores.push_back(Semaphore::Create(pDevice));
		m_renderDoneSemahpres.push_back(Semaphore::Create(pDevice));
	}

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
	m_pThreadTaskQueue->WaitForFree();

	std::unique_lock<std::mutex> lock(m_mutex);
	WaitForGPUWork(m_currentFrameIndex);
}

// Increase bin index, frame manager moves to next bin, this function is called at the very beginning of a frame
// This function will make sure that current frame will perform correctly, since all previous dependency work has been finished
// This function will make sure that a lot of swapchain acquire will be available after calling this function
// The key is that if the count of previous cached work is more than (max frame count - 2), the oldest one needs to be flushed
// or swapchain acquire will fail
// 2 comes from min frame count of swapchain
void FrameManager::BeforeAcquire()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_currentSemaphoreIndex = (m_currentSemaphoreIndex + 1) % m_maxFrameCount;
}

void FrameManager::AfterAcquire(uint32_t index)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	WaitForGPUWork(index);
	m_currentFrameIndex = index;
}

void FrameManager::CacheSubmissioninfo(
	const std::shared_ptr<Queue>& pQueue,
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
	const std::vector<VkPipelineStageFlags>& waitStages,
	bool waitUtilQueueIdle)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	
	uint32_t frameIndex = cmdBuffer[0]->GetCommandPool()->GetPerFrameResource().lock()->GetFrameIndex();
	CacheSubmissioninfoInternal(pQueue, cmdBuffer, { m_acquireDoneSemaphores[m_currentSemaphoreIndex] }, waitStages, { m_renderDoneSemahpres[m_currentSemaphoreIndex] }, waitUtilQueueIdle);
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
}

// Add job to current frame
void FrameManager::AddJobToFrame(ThreadJobFunc jobFunc)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_pThreadTaskQueue->AddJob(jobFunc, FrameIndex());
}

// Wait until those gpu work of this frame finished
void FrameManager::WaitForGPUWork(uint32_t frameIndex)
{
	WaitForFence(frameIndex);
	m_submissionInfoTable[frameIndex].clear();
}

// End work submission, which means that current frame's work has been submitted completely
void FrameManager::EndJobSubmission()
{
	m_pThreadTaskQueue->WaitForFree();

	std::unique_lock<std::mutex> lock(m_mutex);
	// Flush cached submission after all cpu work done
	FlushCachedSubmission(m_currentFrameIndex);
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