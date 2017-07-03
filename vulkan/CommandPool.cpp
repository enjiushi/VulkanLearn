#include "CommandPool.h"
#include "CommandBuffer.h"

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(GetDevice()->GetDeviceHandle(), m_commandPool, nullptr);
}

bool CommandPool::Init(const std::shared_ptr<Device>& pDevice, const VkCommandPoolCreateInfo& info)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	CHECK_VK_ERROR(vkCreateCommandPool(pDevice->GetDeviceHandle(), &info, nullptr, &m_commandPool));
	m_info = info;

	return true;
}

std::shared_ptr<CommandBuffer> CommandPool::AllocatePrimaryCommandBuffer(const std::shared_ptr<CommandPool>& pCmdPool)
{
	return CommandBuffer::Create(pCmdPool->GetDevice(), pCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

std::shared_ptr<CommandPool> CommandPool::Create(const std::shared_ptr<Device>& pDevice, const VkCommandPoolCreateInfo& info)
{
	std::shared_ptr<CommandPool> pCommandPool = std::make_shared<CommandPool>();
	if (pCommandPool.get() && pCommandPool->Init(pDevice, info))
		return pCommandPool;
	return nullptr;
}