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
#include <algorithm>

bool FrameManager::Init(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount)
{
	for (uint32_t i = 0; i < maxFrameCount; i++)
	{
		m_frameResTable[i] = std::vector<std::shared_ptr<PerFrameResource>>();
		m_frameFences.push_back(Fence::Create(pDevice));
	}

	m_currentFrameIndex = -1;
	m_maxFrameCount = maxFrameCount;
	
	return true;
}

std::shared_ptr<FrameManager> FrameManager::Create(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount)
{
	std::shared_ptr<FrameManager> pFrameManager = std::make_shared<FrameManager>();
	if (pFrameManager.get() && pFrameManager->Init(pDevice, maxFrameCount))
		return pFrameManager;
	return nullptr;
}

std::shared_ptr<PerFrameResource> FrameManager::AllocatePerFrameResource(uint32_t frameIndex)
{
	if (frameIndex < 0 || frameIndex >= m_maxFrameCount)
		return nullptr;

	std::shared_ptr<PerFrameResource> pPerFrameRes = PerFrameResource::Create(GlobalObjects()->GetDevice(), frameIndex);
	m_frameResTable[frameIndex].push_back(pPerFrameRes);
	return pPerFrameRes;
}

void FrameManager::WaitForFence()
{
	WaitForFence(m_currentFrameIndex);
}

void FrameManager::WaitForFence(uint32_t index)
{
	if (index == -1)
		return;

	m_frameFences[index]->Wait();
	m_frameFences[index]->Reset();
}

void FrameManager::SetFrameIndex(uint32_t index)
{
	if (m_currentFrameIndex == index)
		return;

	// Flush cached submissions before frame changed
	// This is the very place that all of cmd buffer submissions fire
	FlushCachedSubmission();
	m_currentFrameIndex = index % m_maxFrameCount;
	WaitForFence();

	// New frame's running resources are no longer in use now, we've already finished waiting for their protective fence
	// They're cleared and okay to remove
	m_submissionInfoTable[m_currentFrameIndex].clear();
}

void FrameManager::CacheSubmissioninfo(
	const std::shared_ptr<Queue>& pQueue,
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
	bool waitUtilQueueIdle)
{
	SubmissionInfo info = 
	{
		pQueue,
		cmdBuffer,
		waitSemaphores,
		waitStages,
		signalSemaphores,
		waitUtilQueueIdle,
	};

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
	m_pendingSubmissionInfoTable[frameIndex].push_back(info);
}

void FrameManager::FlushCachedSubmission()
{
	if (m_currentFrameIndex == -1)
		return;

	// Flush pending cmd buffers
	std::for_each(m_pendingSubmissionInfoTable[m_currentFrameIndex].begin(), m_pendingSubmissionInfoTable[m_currentFrameIndex].end(), [this](SubmissionInfo & info)
	{
		info.pQueue->SubmitCommandBuffers(info.cmdBuffers, info.waitSemaphores, info.waitStages, info.signalSemaphores, GetCurrentFrameFence(), info.waitUtilQueueIdle);
	});

	// Add submitted cmd buffer references here, just to make sure they won't be deleted util this submission finished
	m_submissionInfoTable[m_currentFrameIndex].insert(
		m_submissionInfoTable[m_currentFrameIndex].end(), 
		m_pendingSubmissionInfoTable[m_currentFrameIndex].begin(), 
		m_pendingSubmissionInfoTable[m_currentFrameIndex].end());

	// Clear pending submissions
	m_pendingSubmissionInfoTable[m_currentFrameIndex].clear();
}