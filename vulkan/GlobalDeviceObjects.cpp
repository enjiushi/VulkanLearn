#include "GlobalDeviceObjects.h"
#include "Queue.h"
#include "CommandPool.h"
#include "DeviceMemoryManager.h"
#include "StagingBufferManager.h"
#include "SwapChain.h"
#include "../thread/ThreadWorker.hpp"
#include "SharedBufferManager.h"
#include "Framebuffer.h"
#include "RenderPass.h"
#include "../thread/ThreadTaskQueue.hpp"
#include "GlobalVulkanStates.h"
#include "PhysicalDevice.h"
#include "PerFrameResource.h"

bool GlobalDeviceObjects::InitObjects(const std::shared_ptr<Device>& pDevice)
{
	m_pDevice = pDevice;

	m_pGraphicQueue = Queue::Create(pDevice, pDevice->GetPhysicalDevice()->GetGraphicQueueIndex());
	m_pComputeQueue = Queue::Create(pDevice, pDevice->GetPhysicalDevice()->GetComputeQueueIndex());
	m_pTransferQueue = Queue::Create(pDevice, pDevice->GetPhysicalDevice()->GetTransferQueueIndex());
	m_pPresentQueue = Queue::Create(pDevice, pDevice->GetPhysicalDevice()->GetPresentQueueIndex());

	m_pMainThreadGraphicCmdPool = CommandPool::Create(pDevice, m_pDevice->GetPhysicalDevice()->GetGraphicQueueIndex());
	m_pMainThreadComputeCmdPool = CommandPool::Create(pDevice, m_pDevice->GetPhysicalDevice()->GetComputeQueueIndex());
	m_pMainThreadTransferCmdPool = CommandPool::Create(pDevice, m_pDevice->GetPhysicalDevice()->GetTransferQueueIndex());

	if (m_pDeviceMemMgr == nullptr)
		m_pDeviceMemMgr = DeviceMemoryManager::Create(pDevice);

	m_pStaingBufferMgr = StagingBufferManager::Create(pDevice);

	m_pIndexBufferMgr = SharedBufferManager::Create(pDevice, 
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		INDEX_BUFFER_SIZE);

	m_pUniformBufferMgr = SharedBufferManager::Create(pDevice, 
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
		(VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), 
		UNIFORM_BUFFER_SIZE);

	m_pShaderStorageBufferMgr = SharedBufferManager::Create(pDevice, 
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
		(VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), 
		SHADER_STORAGE_BUFFER_SIZE);

	m_pIndirectBufferMgr = SharedBufferManager::Create(pDevice, 
		VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, 
		(VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), 
		INDIRECT_BUFFER_SIZE);

	m_pStreamingBufferMgr = SharedBufferManager::Create(pDevice, 
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		(VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), 
		ATTRIBUTE_BUFFER_SIZE);

	m_pSwapChain = SwapChain::Create(pDevice);

	m_pThreadTaskQueue = std::make_shared<ThreadTaskQueue>(pDevice, m_pSwapChain->GetSwapChainImageCount());

	m_pGlobalVulkanStates = GlobalVulkanStates::Create(pDevice);

	return true;
}

GlobalDeviceObjects* GlobalObjects()
{
	return GlobalDeviceObjects::GetInstance();
}

GlobalDeviceObjects::~GlobalDeviceObjects()
{
}

bool GlobalDeviceObjects::RequestAttributeBuffer(uint32_t size, uint32_t& offset)
{
	ASSERTION(m_attributeBufferOffset + size < ATTRIBUTE_BUFFER_SIZE);
	offset = m_attributeBufferOffset;
	m_attributeBufferOffset += size;
	return true;
}

const std::shared_ptr<SharedBufferManager> GlobalDeviceObjects::GetVertexAttribBufferMgr(uint32_t vertexFormat) 
{ 
	if (m_vertexAttribBufferMgrs.find(vertexFormat) == m_vertexAttribBufferMgrs.end())
		m_vertexAttribBufferMgrs[vertexFormat] = SharedBufferManager::Create(m_pDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ATTRIBUTE_BUFFER_SIZE);

	return m_vertexAttribBufferMgrs[vertexFormat];
}

std::shared_ptr<Queue> GlobalGraphicQueue() { return GlobalObjects()->GetGraphicQueue(); }
std::shared_ptr<Queue> GlobalComputeQueue() { return GlobalObjects()->GetComputeQueue(); }
std::shared_ptr<Queue> GlobalTransferQueue() { return GlobalObjects()->GetTransferQueue(); }
std::shared_ptr<Queue> GlobalPresentQueue() { return GlobalObjects()->GetPresentQueue(); }
std::shared_ptr<CommandPool> MainThreadGraphicPool() { return GlobalObjects()->GetMainThreadGraphicCmdPool(); }
std::shared_ptr<CommandPool> MainThreadComputePool() { return GlobalObjects()->GetMainThreadComputeCmdPool(); }
std::shared_ptr<CommandPool> MainThreadTransferPool() { return GlobalObjects()->GetMainThreadTransferCmdPool(); }
std::shared_ptr<DeviceMemoryManager> DeviceMemMgr() { return GlobalObjects()->GetDeviceMemMgr(); }
std::shared_ptr<StagingBufferManager> StagingBufferMgr() { return GlobalObjects()->GetStagingBufferMgr(); }
std::shared_ptr<SwapChain> GetSwapChain() { return GlobalObjects()->GetSwapChain(); }
std::shared_ptr<Device> GetDevice() { return GlobalObjects()->GetDevice(); }
std::shared_ptr<PhysicalDevice> GetPhysicalDevice() { return GetDevice()->GetPhysicalDevice(); }
std::shared_ptr<SharedBufferManager> VertexAttribBufferMgr(uint32_t vertexFormat) { return GlobalObjects()->GetVertexAttribBufferMgr(vertexFormat); }
std::shared_ptr<SharedBufferManager> IndexBufferMgr() { return GlobalObjects()->GetIndexBufferMgr(); }
std::shared_ptr<SharedBufferManager> UniformBufferMgr() { return GlobalObjects()->GetUniformBufferMgr(); }
std::shared_ptr<SharedBufferManager> ShaderStorageBufferMgr() { return GlobalObjects()->GetShaderStorageBufferMgr(); }
std::shared_ptr<SharedBufferManager> IndirectBufferMgr() { return GlobalObjects()->GetIndirectBufferMgr(); }
std::shared_ptr<SharedBufferManager> StreamingBufferMgr() { return GlobalObjects()->GetStreamingBufferMgr(); }
std::shared_ptr<ThreadTaskQueue> GlobalThreadTaskQueue() { return GlobalObjects()->GetThreadTaskQueue(); }
std::shared_ptr<GlobalVulkanStates> GetGlobalVulkanStates() { return GlobalObjects()->GetGlobalVulkanStates(); }