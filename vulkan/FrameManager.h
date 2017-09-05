#pragma once

#include "DeviceObjectBase.h"
#include <map>
#include <functional>
#include <mutex>
#include <deque>
#include "../thread/ThreadWorker.hpp"

class CommandBuffer;
class Fence;
class PerFrameResource;
class CommandBuffer;
class Semaphore;
class Queue;
class ThreadTaskQueue;

class FrameManager : public DeviceObjectBase<FrameManager>
{
	typedef struct _SubmissionInfo
	{
		std::shared_ptr<Queue>						pQueue;
		std::vector<std::shared_ptr<CommandBuffer>> cmdBuffers;
		std::vector<std::shared_ptr<Semaphore>>		waitSemaphores;
		std::vector<VkPipelineStageFlags>			waitStages;
		std::vector<std::shared_ptr<Semaphore>>		signalSemaphores;
		bool										waitUtilQueueIdle;
		bool										submitted;
	}SubmissionInfo;

	typedef std::map<uint32_t, std::vector<std::shared_ptr<PerFrameResource>>> FrameResourceTable;
	typedef std::map<uint32_t, std::vector<SubmissionInfo>> SubmissionInfoTable;

public:
	std::shared_ptr<PerFrameResource> AllocatePerFrameResource(uint32_t frameIndex);
	uint32_t FrameIndex() const { return m_currentFrameIndex; }
	uint32_t MaxFrameCount() const { return m_maxFrameCount; }

	void CacheSubmissioninfo(
		const std::shared_ptr<Queue>& pQueue,
		const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
		const std::vector<VkPipelineStageFlags>& waitStages,
		bool waitUtilQueueIdle);

	void CacheSubmissioninfo(
		const std::shared_ptr<Queue>& pQueue,
		const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
		bool waitUtilQueueIdle);

	// Thread related
	void AddJobToFrame(ThreadJobFunc jobFunc);
	void BeforeAcquire();
	void AfterAcquire(uint32_t index);

	void WaitForAllJobsDone();

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount, const std::shared_ptr<FrameManager>& pSelf);
	static std::shared_ptr<FrameManager> Create(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount);

	std::shared_ptr<Fence> GetCurrentFrameFence() const { return m_frameFences[m_currentFrameIndex]; }
	std::shared_ptr<Fence> GetFrameFence(uint32_t frameIndex) const { return m_frameFences[frameIndex]; }
	void WaitForFence();
	void WaitForFence(uint32_t frameIndex);

	void FlushCachedSubmission(uint32_t frameIndex);
	void EndJobSubmission();

	void WaitForGPUWork(uint32_t frameIndex);

	std::shared_ptr<Semaphore> GetAcqurieDoneSemaphore() const;
	std::shared_ptr<Semaphore> GetAcqurieDoneSemaphore(uint32_t frameIndex) const;
	std::shared_ptr<Semaphore> GetRenderDoneSemaphore();
	std::vector<std::shared_ptr<Semaphore>> GetRenderDoneSemaphores();

	void CacheSubmissioninfoInternal(
		const std::shared_ptr<Queue>& pQueue,
		const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
		bool waitUtilQueueIdle);

private:
	FrameResourceTable						m_frameResTable;
	std::vector<std::shared_ptr<Fence>>		m_frameFences;
	std::vector<std::shared_ptr<Semaphore>>	m_acquireDoneSemaphores;

	std::vector<std::vector<std::shared_ptr<Semaphore>>>	m_renderDoneSemaphores;
	uint32_t												m_renderDoneSemaphoreIndex;

	uint32_t								m_currentFrameIndex;
	std::deque<uint32_t>					m_frameIndexQueue;
	uint32_t								m_currentSemaphoreIndex;

	SubmissionInfoTable						m_pendingSubmissionInfoTable;
	SubmissionInfoTable						m_submissionInfoTable;

	uint32_t m_maxFrameCount;

	std::mutex										m_mutex;

	friend class SwapChain;
	friend class Queue;
};