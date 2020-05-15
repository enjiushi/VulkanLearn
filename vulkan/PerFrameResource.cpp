#include "PerFrameResource.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "DescriptorSet.h"
#include "Fence.h"
#include "GlobalDeviceObjects.h"
#include "SwapChain.h"

bool PerFrameResource::Init(const std::shared_ptr<Device>& pDevice, uint32_t frameIndex, const std::shared_ptr<PerFrameResource>& pSelf)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	for (uint32_t i = 0; i < (uint32_t)QueueFamily::COUNT; i++)
	{
		for (uint32_t j = 0; j < (uint32_t)CBPersistancy::COUNT; j++)
		{
			QueueFamily queueFamily = (QueueFamily)i;
			CBPersistancy persistancy = (CBPersistancy)j;

			switch (queueFamily)
			{
			case PerFrameResource::QueueFamily::GRAPHIC:
				if (persistancy == CBPersistancy::PERSISTANT)
					m_commandPools[i][j] = CommandPool::Create(pDevice, pDevice->GetPhysicalDevice()->GetGraphicQueueIndex(), pSelf);
				else
					m_commandPools[i][j] = CommandPool::CreateTransientCBPool(pDevice, pDevice->GetPhysicalDevice()->GetGraphicQueueIndex(), pSelf);
				break;
			case PerFrameResource::QueueFamily::COMPUTE:
				if (persistancy == CBPersistancy::PERSISTANT)
					m_commandPools[i][j] = CommandPool::Create(pDevice, pDevice->GetPhysicalDevice()->GetComputeQueueIndex(), pSelf);
				else
					m_commandPools[i][j] = CommandPool::CreateTransientCBPool(pDevice, pDevice->GetPhysicalDevice()->GetComputeQueueIndex(), pSelf);
				break;
			case PerFrameResource::QueueFamily::TRASFER:
				// FIXME: Do dedicated transfer queue later
				break;
			default:
				ASSERTION(false);
				break;
			}
		}
	}

	m_frameIndex = frameIndex;

	return true;
}

std::shared_ptr<PerFrameResource> PerFrameResource::Create(const std::shared_ptr<Device>& pDevice, uint32_t frameBinIndex)
{
	std::shared_ptr<PerFrameResource> pPerFrameRes = std::make_shared<PerFrameResource>();
	if (pPerFrameRes.get() && pPerFrameRes->Init(pDevice, frameBinIndex, pPerFrameRes))
		return pPerFrameRes;
	return nullptr;
}

std::shared_ptr<CommandBuffer> PerFrameResource::AllocateCommandBuffer(QueueFamily queueFamily, CBPersistancy persistancy, CBLevel level)
{
	if (level == CBLevel::PRIMARY)
		return m_commandPools[(uint32_t)queueFamily][(uint32_t)persistancy]->AllocatePrimaryCommandBuffer();
	else
		return m_commandPools[(uint32_t)queueFamily][(uint32_t)persistancy]->AllocateSecondaryCommandBuffer();
}