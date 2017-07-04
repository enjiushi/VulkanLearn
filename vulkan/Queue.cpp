#include "Queue.h"
#include "CommandBuffer.h"
#include "Semaphore.h"

Queue::~Queue()
{
}

bool Queue::Init(const std::shared_ptr<Device>& pDevice, uint32_t queueIndex)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	vkGetDeviceQueue(pDevice->GetDeviceHandle(), pDevice->GetPhysicalDevice()->GetGraphicQueueIndex(), 0, &m_queue);
	return true;
}

std::shared_ptr<Queue> Queue::Create(const std::shared_ptr<Device>& pDevice, uint32_t queueIndex)
{
	std::shared_ptr<Queue> pQueue = std::make_shared<Queue>();
	if (pQueue.get() && pQueue->Init(pDevice, queueIndex))
		return pQueue;
	return nullptr;
}

void Queue::SubmitCommandBuffer(const std::shared_ptr<CommandBuffer>& pCmdBuffer, bool waitUtilQueueIdle)
{
	std::vector<std::shared_ptr<CommandBuffer>> v = { pCmdBuffer };
	SubmitCommandBuffers(v, waitUtilQueueIdle);
}

void Queue::SubmitCommandBuffers(const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers, bool waitUtilQueueIdle)
{
	SubmitCommandBuffers(cmdBuffers, std::vector<std::shared_ptr<Semaphore>>(), std::vector<VkPipelineStageFlags>(), waitUtilQueueIdle);
}

void Queue::SubmitCommandBuffer(
	const std::shared_ptr<CommandBuffer>& pCmdBuffer,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	bool waitUtilQueueIdle)
{
	std::vector<std::shared_ptr<CommandBuffer>> v = { pCmdBuffer };
	SubmitCommandBuffers(v, waitSemaphores, waitStages, waitUtilQueueIdle);
}

void Queue::SubmitCommandBuffers(
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	bool waitUtilQueueIdle)
{
	SubmitCommandBuffers(cmdBuffers, std::vector<std::shared_ptr<Semaphore>>(), std::vector<VkPipelineStageFlags>(), std::vector<std::shared_ptr<Semaphore>>(), waitUtilQueueIdle);
}

void Queue::SubmitCommandBuffer(
	const std::shared_ptr<CommandBuffer>& pCmdBuffer,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
	bool waitUtilQueueIdle)
{
	std::vector<std::shared_ptr<CommandBuffer>> v = { pCmdBuffer };
	SubmitCommandBuffers(v, waitSemaphores, waitStages, signalSemaphores, waitUtilQueueIdle);
}

void Queue::SubmitCommandBuffers(
	const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers,
	const std::vector<std::shared_ptr<Semaphore>>& waitSemaphores,
	const std::vector<VkPipelineStageFlags>& waitStages,
	const std::vector<std::shared_ptr<Semaphore>>& signalSemaphores,
	bool waitUtilQueueIdle)
{
	std::vector<VkCommandBuffer> deviceCmdBuffers;
	for (uint32_t i = 0; i < cmdBuffers.size(); i++)
		deviceCmdBuffers.push_back(cmdBuffers[i]->GetDeviceHandle());

	std::vector<VkSemaphore> deviceWaitSemaphores;
	for (uint32_t i = 0; i < waitSemaphores.size(); i++)
		deviceWaitSemaphores.push_back(waitSemaphores[i]->GetDeviceHandle());

	std::vector<VkSemaphore> deviceSignalSemaphores;
	for (uint32_t i = 0; i < signalSemaphores.size(); i++)
		deviceSignalSemaphores.push_back(signalSemaphores[i]->GetDeviceHandle());

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount	= deviceCmdBuffers.size();
	submitInfo.pCommandBuffers		= deviceCmdBuffers.data();
	submitInfo.waitSemaphoreCount	= deviceWaitSemaphores.size();
	submitInfo.pWaitSemaphores		= deviceWaitSemaphores.data();
	submitInfo.pWaitDstStageMask	= waitStages.data();
	submitInfo.signalSemaphoreCount = deviceSignalSemaphores.size();
	submitInfo.pSignalSemaphores	= deviceSignalSemaphores.data();
	CHECK_VK_ERROR(vkQueueSubmit(GetDeviceHandle(), 1, &submitInfo, nullptr));

	if (waitUtilQueueIdle)
		CHECK_VK_ERROR(vkQueueWaitIdle(GetDeviceHandle()));
}