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

bool GlobalDeviceObjects::InitObjects(const std::shared_ptr<Device>& pDevice)
{
	m_pDevice = pDevice;

	for (uint32_t i = 0; i < (uint32_t)PhysicalDevice::QueueFamily::COUNT; i++)
	{
		m_queues[i] = Queue::Create(pDevice, (PhysicalDevice::QueueFamily)i);
		m_pMainThreadCommandPools[i] = CommandPool::Create(pDevice, PhysicalDevice::QueueFamily::ALL_ROUND, CommandPool::CBPersistancy::PERSISTANT);
	}

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

const std::shared_ptr<SharedBufferManager>& GlobalDeviceObjects::GetVertexAttribBufferMgr(uint32_t vertexFormat) 
{ 
	if (m_vertexAttribBufferMgrs.find(vertexFormat) == m_vertexAttribBufferMgrs.end())
		m_vertexAttribBufferMgrs[vertexFormat] = SharedBufferManager::Create(m_pDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ATTRIBUTE_BUFFER_SIZE);

	return m_vertexAttribBufferMgrs[vertexFormat];
}

const std::shared_ptr<CommandPool>& MainThreadCommandPool(PhysicalDevice::QueueFamily queueFamily) { return GlobalObjects()->GetMainThreadCommandPool(queueFamily); }
const std::shared_ptr<DeviceMemoryManager>& DeviceMemMgr() { return GlobalObjects()->GetDeviceMemMgr(); }
const std::shared_ptr<StagingBufferManager>& StagingBufferMgr() { return GlobalObjects()->GetStagingBufferMgr(); }
const std::shared_ptr<SwapChain>& GetSwapChain() { return GlobalObjects()->GetSwapChain(); }
const std::shared_ptr<Device>& GetDevice() { return GlobalObjects()->GetDevice(); }
std::shared_ptr<PhysicalDevice> GetPhysicalDevice() { return GetDevice()->GetPhysicalDevice(); }
const std::shared_ptr<SharedBufferManager>& VertexAttribBufferMgr(uint32_t vertexFormat) { return GlobalObjects()->GetVertexAttribBufferMgr(vertexFormat); }
const std::shared_ptr<SharedBufferManager>& IndexBufferMgr() { return GlobalObjects()->GetIndexBufferMgr(); }
const std::shared_ptr<SharedBufferManager>& UniformBufferMgr() { return GlobalObjects()->GetUniformBufferMgr(); }
const std::shared_ptr<SharedBufferManager>& ShaderStorageBufferMgr() { return GlobalObjects()->GetShaderStorageBufferMgr(); }
const std::shared_ptr<SharedBufferManager>& IndirectBufferMgr() { return GlobalObjects()->GetIndirectBufferMgr(); }
const std::shared_ptr<SharedBufferManager>& StreamingBufferMgr() { return GlobalObjects()->GetStreamingBufferMgr(); }
const std::shared_ptr<ThreadTaskQueue>& GlobalThreadTaskQueue() { return GlobalObjects()->GetThreadTaskQueue(); }
const std::shared_ptr<GlobalVulkanStates>& GetGlobalVulkanStates() { return GlobalObjects()->GetGlobalVulkanStates(); }