#pragma once

#include "DeviceObjectBase.h"
#include <map>

class CommandBuffer;
class Fence;
class PerFrameResource;
class CommandBuffer;
class Semaphore;
class Queue;

class FrameManager
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

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount);
	static std::shared_ptr<FrameManager> Create(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount);

	std::shared_ptr<Fence> GetCurrentFrameFence() const { return m_frameFences[m_currentFrameIndex]; }
	void WaitForFence();
	void CacheSubmissioninfo(
		const std::shared_ptr<Queue>& pQueue,
		const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffer,
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
		bool waitUtilQueueIdle);
	void FlushCachedSubmission();
	void SetFrameIndex(uint32_t index);

private:
	FrameResourceTable						m_frameResTable;
	std::vector<std::shared_ptr<Fence>>		m_frameFences;

	SubmissionInfoTable						m_pendingSubmissionInfoTable;
	SubmissionInfoTable						m_submissionInfoTable;

	uint32_t m_currentFrameIndex;
	uint32_t m_maxFrameCount;

	friend class SwapChain;
	friend class Queue;
};