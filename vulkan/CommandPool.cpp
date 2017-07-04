#include "CommandPool.h"
#include "CommandBuffer.h"
#include "GlobalDeviceObjects.h"

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(GetDevice()->GetDeviceHandle(), m_commandPool, nullptr);
}

bool CommandPool::Init(const std::shared_ptr<Device>& pDevice)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	m_info.queueFamilyIndex = m_pDevice->GetPhysicalDevice()->GetGraphicQueueIndex();
	m_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	CHECK_VK_ERROR(vkCreateCommandPool(pDevice->GetDeviceHandle(), &m_info, nullptr, &m_commandPool));

	return true;
}

std::shared_ptr<CommandBuffer> CommandPool::AllocatePrimaryCommandBuffer(const std::shared_ptr<CommandPool>& pCmdPool)
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = CommandBuffer::Create(pCmdPool->GetDevice(), pCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	pCmdPool->m_cmdBufferList.push_back(pCmdBuffer);
	return pCmdBuffer;
}

std::vector<std::shared_ptr<CommandBuffer>> CommandPool::AllocatePrimaryCommandBuffers(const std::shared_ptr<CommandPool>& pCmdPool, uint32_t count)
{
	std::vector<std::shared_ptr<CommandBuffer>> cmdBuffers;
	for (uint32_t i = 0; i < count; i++)
		cmdBuffers.push_back(AllocatePrimaryCommandBuffer(pCmdPool));

	return cmdBuffers;
}

std::shared_ptr<CommandPool> CommandPool::Create(const std::shared_ptr<Device>& pDevice)
{
	std::shared_ptr<CommandPool> pCommandPool = std::make_shared<CommandPool>();
	if (pCommandPool.get() && pCommandPool->Init(pDevice))
		return pCommandPool;
	return nullptr;
}