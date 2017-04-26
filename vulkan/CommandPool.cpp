#include "CommandPool.h"

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

VkCommandBuffer CommandPool::AllocateCommandBuffer()
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = m_commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuffer;
	CHECK_VK_ERROR(vkAllocateCommandBuffers(m_pDevice->GetDeviceHandle(), &commandBufferAllocateInfo, &cmdBuffer));

	return cmdBuffer;
}

std::shared_ptr<CommandPool> CommandPool::Create(const std::shared_ptr<Device>& pDevice, const VkCommandPoolCreateInfo& info)
{
	std::shared_ptr<CommandPool> pCommandPool = std::make_shared<CommandPool>();
	if (pCommandPool.get() && pCommandPool->Init(pDevice, info))
		return pCommandPool;
	return nullptr;
}