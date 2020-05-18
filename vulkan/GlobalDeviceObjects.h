#pragma once

#include "../common/Singleton.h"
#include "Device.h"
#include <map>

class Queue;
class CommandPool;
class DeviceMemoryManager;
class StagingBufferManager;
class Buffer;
class SwapChain;
class PhysicalDevice;
class SharedBufferManager;
class FrameBuffer;
class ThreadTaskQueue;
class GlobalVulkanStates;
class PerFrameResource;
class RenderPass;

class GlobalDeviceObjects;

GlobalDeviceObjects* GlobalObjects();
const std::shared_ptr<CommandPool>& MainThreadCommandPool(PhysicalDevice::QueueFamily queueFamily);
const std::shared_ptr<DeviceMemoryManager>& DeviceMemMgr();
const std::shared_ptr<StagingBufferManager>& StagingBufferMgr();
const std::shared_ptr<SwapChain>& GetSwapChain();
const std::shared_ptr<Device>& GetDevice();
std::shared_ptr<PhysicalDevice> GetPhysicalDevice();
const std::shared_ptr<SharedBufferManager>& VertexAttribBufferMgr(uint32_t vertexFormat);
const std::shared_ptr<SharedBufferManager>& IndexBufferMgr();
const std::shared_ptr<SharedBufferManager>& UniformBufferMgr();
const std::shared_ptr<SharedBufferManager>& ShaderStorageBufferMgr();
const std::shared_ptr<SharedBufferManager>& IndirectBufferMgr();
const std::shared_ptr<SharedBufferManager>& StreamingBufferMgr();
const std::shared_ptr<ThreadTaskQueue>& GlobalThreadTaskQueue();
const std::shared_ptr<GlobalVulkanStates>& GetGlobalVulkanStates();

class GlobalDeviceObjects : public Singleton<GlobalDeviceObjects>
{
public:
	bool InitObjects(const std::shared_ptr<Device>& pDevice);

	~GlobalDeviceObjects();

public:
	const std::shared_ptr<Device>& GetDevice() const { return m_pDevice; }
	const std::shared_ptr<Queue>& GetQueue(PhysicalDevice::QueueFamily queueFamily) const { return m_queues[(uint32_t)queueFamily]; }
	const std::shared_ptr<CommandPool>& GetMainThreadCommandPool(PhysicalDevice::QueueFamily queueFamily) { return m_pMainThreadCommandPools[(uint32_t)queueFamily]; }
	const std::shared_ptr<DeviceMemoryManager>& GetDeviceMemMgr() const { return m_pDeviceMemMgr; }
	const std::shared_ptr<StagingBufferManager>& GetStagingBufferMgr() const { return m_pStaingBufferMgr; }
	const std::shared_ptr<SwapChain>& GetSwapChain() const { return m_pSwapChain; }
	const std::shared_ptr<SharedBufferManager>& GetVertexAttribBufferMgr(uint32_t vertexFormat);
	const std::shared_ptr<SharedBufferManager>& GetIndexBufferMgr() const { return m_pIndexBufferMgr; }
	const std::shared_ptr<SharedBufferManager>& GetUniformBufferMgr() const { return m_pUniformBufferMgr; }
	const std::shared_ptr<SharedBufferManager>& GetShaderStorageBufferMgr() const { return m_pShaderStorageBufferMgr; }
	const std::shared_ptr<SharedBufferManager>& GetIndirectBufferMgr() const { return m_pIndirectBufferMgr; }
	const std::shared_ptr<SharedBufferManager>& GetStreamingBufferMgr() const { return m_pStreamingBufferMgr; }
	const std::shared_ptr<ThreadTaskQueue>& GetThreadTaskQueue() const { return m_pThreadTaskQueue; }
	const std::shared_ptr<GlobalVulkanStates>& GetGlobalVulkanStates() const { return m_pGlobalVulkanStates; }

	//FIXME : remove me
	bool RequestAttributeBuffer(uint32_t size, uint32_t& offset);

protected:
	std::shared_ptr<Device>					m_pDevice;
	std::shared_ptr<Queue>					m_queues[(uint32_t)PhysicalDevice::QueueFamily::COUNT];
	std::shared_ptr<CommandPool>			m_pMainThreadCommandPools[(uint32_t)PhysicalDevice::QueueFamily::COUNT];
	std::shared_ptr<DeviceMemoryManager>	m_pDeviceMemMgr;

	std::shared_ptr<StagingBufferManager>	m_pStaingBufferMgr;

	std::shared_ptr<SwapChain>				m_pSwapChain;

	std::shared_ptr<SharedBufferManager>	m_pIndexBufferMgr;
	std::shared_ptr<SharedBufferManager>	m_pUniformBufferMgr;
	std::shared_ptr<SharedBufferManager>	m_pShaderStorageBufferMgr;
	std::shared_ptr<SharedBufferManager>	m_pIndirectBufferMgr;
	std::shared_ptr<SharedBufferManager>	m_pStreamingBufferMgr;

	// Key: vertex format, value: vertex buffer manager
	std::map<uint32_t, std::shared_ptr<SharedBufferManager>>	m_vertexAttribBufferMgrs;

	std::shared_ptr<GlobalVulkanStates>		m_pGlobalVulkanStates;

	std::shared_ptr<ThreadTaskQueue>		m_pThreadTaskQueue;

	static const uint32_t ATTRIBUTE_BUFFER_SIZE = 1024 * 1024 * 64;
	static const uint32_t INDEX_BUFFER_SIZE = 1024 * 1024 * 4;
	static const uint32_t UNIFORM_BUFFER_SIZE = 1024 * 512;
	static const uint32_t SHADER_STORAGE_BUFFER_SIZE = 1024 * 1024 * 2;
	static const uint32_t INDIRECT_BUFFER_SIZE = 1024 * 1024;

	uint32_t								m_attributeBufferOffset = 0;
};