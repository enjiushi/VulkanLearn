#include "FrameWorkManager.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/CommandPool.h"
#include "../vulkan/DescriptorPool.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/Fence.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/PerFrameResource.h"
#include "../vulkan/Fence.h"
#include "../vulkan/Semaphore.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/Queue.h"
#include "../thread/ThreadTaskQueue.hpp"
#include <algorithm>
#include "../vulkan/Semaphore.h"
#include <stack>

bool FrameWorkManager::Init()
{
	m_currentSemaphoreIndex = 0;
	m_maxFrameCount = GetSwapChain()->GetSwapChainImageCount();

	for (uint32_t i = 0; i < m_maxFrameCount; i++)
	{
		m_frameResTable[i] = std::vector<std::shared_ptr<PerFrameResource>>();
		m_frameFences.push_back(Fence::Create(GetDevice()));
		m_extraFrameFences.push_back(ExtraFences::Create());
		m_acquireDoneSemaphores.push_back(Semaphore::Create(GetDevice()));
	}

	m_renderDoneSemaphores.resize(m_maxFrameCount);
	m_renderDoneSemaphoreIndex = 0;

	for (uint32_t i = 0; i < m_maxFrameCount; i++)
		m_mainThreadPerFrameRes.push_back(FrameWorkManager::GetInstance()->AllocatePerFrameResource(i));
	
	return true;
}

std::shared_ptr<FrameWorkManager::ExtraFences> FrameWorkManager::ExtraFences::Create()
{
	std::shared_ptr<FrameWorkManager::ExtraFences> pExtraFences = std::make_shared<FrameWorkManager::ExtraFences>();
	if (pExtraFences.get() && pExtraFences->Init(pExtraFences))
		return pExtraFences;

	return nullptr;
}

bool FrameWorkManager::ExtraFences::Init(const std::shared_ptr<ExtraFences>& pExtraFences)
{
	if (!SelfRefBase::Init(pExtraFences))
		return false;

	m_currIndex = -1;
	return true;
}

std::shared_ptr<Fence> FrameWorkManager::ExtraFences::GetCurrentFence()
{
	if (m_currIndex < 0)
		return GetNewFence();

	return m_fences[m_currIndex];
}

std::shared_ptr<Fence> FrameWorkManager::ExtraFences::GetNewFence()
{
	m_currIndex++;
	if ((int32_t)m_fences.size() == m_currIndex)
		m_fences.push_back(Fence::Create(GlobalObjects()->GetDevice()));
	else
		m_fences[m_currIndex]->Reset();
	return m_fences[m_currIndex];
}

void FrameWorkManager::ExtraFences::Reset()
{
	for (int32_t i = 0; i <= m_currIndex; i++)
	{
		m_fences[i]->Reset();
		m_fences[i]->ClearReferenceTable();
	}

	m_currIndex = -1;
}

void FrameWorkManager::ExtraFences::Wait()
{
	for (int32_t i = 0; i <= m_currIndex; i++)
		m_fences[i]->Wait();
}

std::shared_ptr<PerFrameResource> FrameWorkManager::AllocatePerFrameResource(uint32_t frameBinIndex)
{
	if (frameBinIndex < 0 || frameBinIndex >= m_maxFrameCount)
		return nullptr;

	std::shared_ptr<PerFrameResource> pPerFrameRes = PerFrameResource::Create(GetDevice(), frameBinIndex);
	m_frameResTable[frameBinIndex].push_back(pPerFrameRes);
	return pPerFrameRes;
}

void FrameWorkManager::WaitForFence()
{
	WaitForFence(m_currentFrameIndex);
}

void FrameWorkManager::WaitForFence(uint32_t frameIndex)
{
	m_frameFences[frameIndex]->Wait();
	m_extraFrameFences[frameIndex]->Wait();
	m_extraFrameFences[frameIndex]->Reset();
}

void FrameWorkManager::WaitForAllJobsDone()
{
	GlobalThreadTaskQueue()->WaitForFree();

	std::unique_lock<std::mutex> lock(m_mutex);
	WaitForGPUWork(m_currentFrameIndex);
}

const std::shared_ptr<PerFrameResource> FrameWorkManager::GetMainThreadPerFrameRes() const
{
	return m_mainThreadPerFrameRes[FrameWorkManager::GetInstance()->FrameIndex()];
}

// Increase bin index, frame manager moves to next bin, this function is called at the very beginning of a frame
// This function will make sure that current frame will perform correctly, since all previous dependency work has been finished
// This function will make sure that a lot of swapchain acquire will be available after calling this function
// The key is that if the count of previous cached work is more than (max frame count - 2), the oldest one needs to be flushed
// or swapchain acquire will fail
// 2 comes from min frame count of swapchain
void FrameWorkManager::BeforeAcquire()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_currentSemaphoreIndex = (m_currentSemaphoreIndex + 1) % m_maxFrameCount;
}

void FrameWorkManager::AfterAcquire(uint32_t index)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	WaitForGPUWork(index);
	m_currentFrameIndex = index;
}

void FrameWorkManager::AcquireNextImage()
{
	BeforeAcquire();

	uint32_t index = GetSwapChain()->AcquireNextImage(GetAcqurieDoneSemaphore());

	AfterAcquire(index);
}

void FrameWorkManager::QueuePresentImage()
{
	// Flush pending submissions before present
	EndJobSubmission();

	GetSwapChain()->QueuePresentImage(GlobalObjects()->GetPresentQueue(), GetRenderDoneSemaphores(), m_currentFrameIndex);
}

void FrameWorkManager::SubmitCommandBuffers(
	const std::shared_ptr<Queue>& pQueue,
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
	const std::vector<VkPipelineStageFlags>& waitStages,
	bool waitUtilQueueIdle,
	bool cache)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	
	SubmitCommandBuffersInternal(pQueue, cmdBuffer, { }, waitStages, { }, waitUtilQueueIdle, cache);
}

void FrameWorkManager::SubmitCommandBuffers(
	const std::shared_ptr<Queue>& pQueue,
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
	bool waitUtilQueueIdle,
	bool cache)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	SubmitCommandBuffersInternal(pQueue, cmdBuffer, waitSemaphores, waitStages, signalSemaphores, waitUtilQueueIdle, cache);
}

void FrameWorkManager::SubmitCommandBuffersInternal(
	const std::shared_ptr<Queue>& pQueue,
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
	bool waitUtilQueueIdle,
	bool cache)
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

	if (cache)
	{
		// Attach acquire done semaphore to waiting list
		std::vector<std::shared_ptr<Semaphore>> _waitSemaphores = waitSemaphores;
		_waitSemaphores.push_back(GetAcqurieDoneSemaphore());

		std::vector<VkPipelineStageFlags> _waitStages;
		_waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		// Attach render done semaphores to signal list
		std::vector<std::shared_ptr<Semaphore>> _signalSemaphores = signalSemaphores;
		_signalSemaphores.insert(_signalSemaphores.end(), signalSemaphores.begin(), signalSemaphores.end());
		_signalSemaphores.push_back(GetRenderDoneSemaphore());

		SubmissionInfo info =
		{
			pQueue,
			cmdBuffer,
			_waitSemaphores,
			_waitStages,
			_signalSemaphores,
			waitUtilQueueIdle,
		};

		m_pendingSubmissionInfoTable[m_currentFrameIndex].push_back(info);
	}
	else
	{
		pQueue->SubmitCommandBuffers(cmdBuffer, waitSemaphores, waitStages, signalSemaphores, m_extraFrameFences[m_currentFrameIndex]->GetNewFence(), waitUtilQueueIdle);

		// Add objects used for this submission into fences reference table
		// They'll be cleared once this submission is done
		for (uint32_t i = 0; i < (uint32_t)cmdBuffer.size(); i++)
			m_extraFrameFences[m_currentFrameIndex]->GetCurrentFence()->AddToReferenceTable(cmdBuffer[i]);

		for (uint32_t i = 0; i < (uint32_t)waitSemaphores.size(); i++)
			m_extraFrameFences[m_currentFrameIndex]->GetCurrentFence()->AddToReferenceTable(waitSemaphores[i]);

		for (uint32_t i = 0; i < (uint32_t)signalSemaphores.size(); i++)
			m_extraFrameFences[m_currentFrameIndex]->GetCurrentFence()->AddToReferenceTable(signalSemaphores[i]);
	}
}

void FrameWorkManager::FlushCachedSubmission(uint32_t frameIndex)
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
void FrameWorkManager::AddJobToFrame(ThreadJobFunc jobFunc)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	GlobalThreadTaskQueue()->AddJob(jobFunc, FrameIndex());
}

// Wait until those gpu work of this frame finished
void FrameWorkManager::WaitForGPUWork(uint32_t frameIndex)
{
	WaitForFence(frameIndex);
	m_submissionInfoTable[frameIndex].clear();
}

// End work submission, which means that current frame's work has been submitted completely
void FrameWorkManager::EndJobSubmission()
{
	GlobalThreadTaskQueue()->WaitForFree();

	std::unique_lock<std::mutex> lock(m_mutex);
	// Flush cached submission after all cpu work done
	FlushCachedSubmission(m_currentFrameIndex);

	// Reset
	m_renderDoneSemaphoreIndex = 0;
}

std::shared_ptr<Semaphore> FrameWorkManager::GetAcqurieDoneSemaphore() const
{
	return m_acquireDoneSemaphores[m_currentSemaphoreIndex]; 
}

std::shared_ptr<Semaphore> FrameWorkManager::GetAcqurieDoneSemaphore(uint32_t frameIndex) const
{
	return m_acquireDoneSemaphores[frameIndex];
}

std::shared_ptr<Semaphore> FrameWorkManager::GetRenderDoneSemaphore()
{
	// Cached semaphores not enough? create a new one
	if (m_renderDoneSemaphoreIndex >= m_renderDoneSemaphores[m_currentFrameIndex].size())
		m_renderDoneSemaphores[m_currentFrameIndex].push_back(Semaphore::Create(GetDevice()));

	return m_renderDoneSemaphores[m_currentFrameIndex][m_renderDoneSemaphoreIndex++];
}

std::vector<std::shared_ptr<Semaphore>> FrameWorkManager::GetRenderDoneSemaphores()
{
	std::vector<std::shared_ptr<Semaphore>> semaphores;
	semaphores.insert(semaphores.end(), m_renderDoneSemaphores[m_currentFrameIndex].begin(), m_renderDoneSemaphores[m_currentFrameIndex].begin() + m_renderDoneSemaphoreIndex + 1);
	return semaphores;
}