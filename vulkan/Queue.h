#pragma once

#include "DeviceObjectBase.h"
#include "PhysicalDevice.h"

class CommandBuffer;
class Semaphore;
class Fence;

class Queue : public DeviceObjectBase<Queue>
{
public:
	~Queue();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Queue>& pSelf, PhysicalDevice::QueueFamily queueFamily);

public:
	VkQueue GetDeviceHandle() { return m_queue; }

	void SubmitCommandBuffer(const std::shared_ptr<CommandBuffer>& pCmdBuffer, const std::shared_ptr<Fence>& pFence, bool waitUtilQueueIdle = false);
	void SubmitCommandBuffers(const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers, const std::shared_ptr<Fence>& pFence, bool waitUtilQueueIdle = false);

	void SubmitCommandBuffer(
		const std::shared_ptr<CommandBuffer>& pCmdBuffer,
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		const std::shared_ptr<Fence>& pFence,
		bool waitUtilQueueIdle = false);
	void SubmitCommandBuffers(
		const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers,
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		const std::shared_ptr<Fence>& pFence,
		bool waitUtilQueueIdle = false);

	void SubmitCommandBuffer(
		const std::shared_ptr<CommandBuffer>& pCmdBuffer,
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
		const std::shared_ptr<Fence>& pFence,
		bool waitUtilQueueIdle = false);
	void SubmitCommandBuffers(
		const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers,
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
		const std::shared_ptr<Fence>& pFence,
		bool waitUtilQueueIdle = false);

	void WaitForIdle();

public:
	PhysicalDevice::QueueFamily GetQueueFamily() const { return m_queueFamily; }

public:
	static std::shared_ptr<Queue> Create(const std::shared_ptr<Device>& pDevice, PhysicalDevice::QueueFamily queueFamily);

protected:
	VkQueue						m_queue;
	PhysicalDevice::QueueFamily m_queueFamily;
};