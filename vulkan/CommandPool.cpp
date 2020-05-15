#include "CommandPool.h"
#include "CommandBuffer.h"
#include "GlobalDeviceObjects.h"
#include "Fence.h"

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(GetDevice()->GetDeviceHandle(), m_commandPool, nullptr);
}

bool CommandPool::Init(const std::shared_ptr<Device>& pDevice, uint32_t queueFamilyIndex, const std::shared_ptr<CommandPool>& pSelf, VkCommandPoolCreateFlags flags)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	m_info.queueFamilyIndex = queueFamilyIndex;
	m_info.flags = flags;
	CHECK_VK_ERROR(vkCreateCommandPool(pDevice->GetDeviceHandle(), &m_info, nullptr, &m_commandPool));

	return true;
}

std::shared_ptr<CommandBuffer> CommandPool::AllocatePrimaryCommandBuffer()
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = CommandBuffer::Create(GetDevice(), GetSelfSharedPtr(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	return pCmdBuffer;
}

std::shared_ptr<CommandBuffer> CommandPool::AllocateSecondaryCommandBuffer()
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = CommandBuffer::Create(GetDevice(), GetSelfSharedPtr(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	return pCmdBuffer;
}

std::vector<std::shared_ptr<CommandBuffer>> CommandPool::AllocatePrimaryCommandBuffers(uint32_t count)
{
	std::vector<std::shared_ptr<CommandBuffer>> cmdBuffers;
	for (uint32_t i = 0; i < count; i++)
		cmdBuffers.push_back(AllocatePrimaryCommandBuffer());

	return cmdBuffers;
}

std::vector<std::shared_ptr<CommandBuffer>> CommandPool::AllocateSecondaryCommandBuffers(uint32_t count)
{
	std::vector<std::shared_ptr<CommandBuffer>> cmdBuffers;
	for (uint32_t i = 0; i < count; i++)
		cmdBuffers.push_back(AllocateSecondaryCommandBuffer());

	return cmdBuffers;
}

std::shared_ptr<CommandPool> CommandPool::Create(const std::shared_ptr<Device>& pDevice, uint32_t queueFamilyIndex, const std::shared_ptr<PerFrameResource>& pPerFrameRes)
{
	std::shared_ptr<CommandPool> pCommandPool = std::make_shared<CommandPool>();
	if (pCommandPool.get() && pCommandPool->Init(pDevice, queueFamilyIndex, pCommandPool))
		return pCommandPool;
	return nullptr;
}

std::shared_ptr<CommandPool> CommandPool::CreateTransientCBPool(const std::shared_ptr<Device>& pDevice, uint32_t queueFamilyIndex, const std::shared_ptr<PerFrameResource>& pPerFrameRes)
{
	std::shared_ptr<CommandPool> pCommandPool = std::make_shared<CommandPool>();
	if (pCommandPool.get() && pCommandPool->Init(pDevice, queueFamilyIndex, pCommandPool, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT))
		return pCommandPool;
	return nullptr;
}

std::shared_ptr<CommandPool> CommandPool::Create(const std::shared_ptr<Device>& pDevice, uint32_t queueFamilyIndex)
{
	return Create(pDevice, queueFamilyIndex, nullptr);
}

void CommandPool::Reset(const std::shared_ptr<Fence>& pFence)
{
	//Hold this fence pointer until the end of this function
	std::shared_ptr<Fence> pFenceHolder = pFence;

	if (pFenceHolder.get())
	{
		VkFence fence = pFenceHolder->GetDeviceHandle();
		CHECK_VK_ERROR(vkWaitForFences(GetDevice()->GetDeviceHandle(), 1, &fence, VK_TRUE, UINT64_MAX));
	}

	vkResetCommandPool(GetDevice()->GetDeviceHandle(), m_commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
}