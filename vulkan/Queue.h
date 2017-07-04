#pragma once

#include "DeviceObjectBase.h"

class CommandBuffer;
class Semaphore;

class Queue : public DeviceObjectBase
{
public:
	~Queue();

	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t queueIndex);

public:
	VkQueue GetDeviceHandle() { return m_queue; }

	void SubmitCommandBuffer(const std::shared_ptr<CommandBuffer>& pCmdBuffer, bool waitUtilQueueIdle = false);
	void SubmitCommandBuffers(const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers, bool waitUtilQueueIdle = false);

	void SubmitCommandBuffer(
		const std::shared_ptr<CommandBuffer>& pCmdBuffer, 
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		bool waitUtilQueueIdle = false);
	void SubmitCommandBuffers(
		const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers, 
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		bool waitUtilQueueIdle = false);

	void SubmitCommandBuffer(
		const std::shared_ptr<CommandBuffer>& pCmdBuffer,
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
		bool waitUtilQueueIdle = false);
	void SubmitCommandBuffers(
		const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers,
		const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
		const std::vector<VkPipelineStageFlags>& waitStages,
		const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
		bool waitUtilQueueIdle = false);

public:
	static std::shared_ptr<Queue> Create(const std::shared_ptr<Device>& pDevice, uint32_t queueIndex);

protected:
	VkQueue m_queue;
};