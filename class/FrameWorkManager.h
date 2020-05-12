#pragma once

#include "../vulkan/DeviceObjectBase.h"
#include "../common/Singleton.h"
#include "../thread/ThreadWorker.hpp"
#include <map>
#include <functional>
#include <mutex>
#include <deque>

class CommandBuffer;
class Fence;
class PerFrameResource;
class CommandBuffer;
class Semaphore;
class Queue;
class ThreadTaskQueue;

class FrameWorkManager : public Singleton<FrameWorkManager>
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
	bool Init();

public:
	std::shared_ptr<PerFrameResource> AllocatePerFrameResource(uint32_t frameIndex);
	uint32_t FrameIndex() const { return m_currentFrameIndex; }
	uint32_t MaxFrameCount() const { return m_maxFrameCount; }

	void SubmitCommandBuffers(
		const std::shared_ptr<Queue>& pQueue,
		const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
		const std::vector<VkPipelineStageFlags>& waitStages,
		bool waitUtilQueueIdle,
		bool cache = true);

	void SubmitCommandBuffers(
		const std::shared_ptr<Queue>& pQueue,
		const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
		bool waitUtilQueueIdle,
		bool cache = true);

	// Thread related
	void AddJobToFrame(ThreadJobFunc jobFunc);
	void BeforeAcquire();
	void AfterAcquire(uint32_t index);

	void AcquireNextImage();
	void QueuePresentImage();

	void WaitForAllJobsDone();

	const std::shared_ptr<PerFrameResource> GetMainThreadPerFrameRes() const;

protected:
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

	void SubmitCommandBuffersInternal(
		const std::shared_ptr<Queue>& pQueue,
		const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
		bool waitUtilQueueIdle,
		bool cache);

private:
	class ExtraFences : public SelfRefBase<ExtraFences>
	{
	public:
		static std::shared_ptr<ExtraFences> Create();

	public:
		std::shared_ptr<Fence> GetCurrentFence();
		std::shared_ptr<Fence> GetNewFence();

		void Wait();
		void Reset();

	protected:
		bool Init(const std::shared_ptr<ExtraFences>& pExtraFences);

	private:
		std::vector<std::shared_ptr<Fence>>		m_fences;
		int32_t									m_currIndex;
	};

	FrameResourceTable							m_frameResTable;
	std::vector<std::shared_ptr<Fence>>			m_frameFences;
	std::vector<std::shared_ptr<ExtraFences>>	m_extraFrameFences;	// For those CBs submitting immediately
	std::vector<std::shared_ptr<Semaphore>>		m_acquireDoneSemaphores;

	std::vector<std::vector<std::shared_ptr<Semaphore>>>	m_renderDoneSemaphores;
	uint32_t												m_renderDoneSemaphoreIndex;

	std::vector<std::shared_ptr<PerFrameResource>>			m_mainThreadPerFrameRes;

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