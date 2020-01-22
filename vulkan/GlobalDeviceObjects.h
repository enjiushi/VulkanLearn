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
class FrameManager;
class PhysicalDevice;
class SharedBufferManager;
class FrameBuffer;
class ThreadTaskQueue;
class GlobalVulkanStates;
class PerFrameResource;
class RenderPass;

class GlobalDeviceObjects;

GlobalDeviceObjects* GlobalObjects();
std::shared_ptr<Queue> GlobalGraphicQueue();
std::shared_ptr<Queue> GlobalComputeQueue();
std::shared_ptr<Queue> GlobalTransferQueue();
std::shared_ptr<Queue> GlobalPresentQueue();
std::shared_ptr<CommandPool> MainThreadGraphicPool();
std::shared_ptr<CommandPool> MainThreadComputePool();
std::shared_ptr<CommandPool> MainThreadTransferPool();
std::shared_ptr<DeviceMemoryManager> DeviceMemMgr();
std::shared_ptr<StagingBufferManager> StagingBufferMgr();
std::shared_ptr<SwapChain> GetSwapChain();
std::shared_ptr<FrameManager> FrameMgr();
std::shared_ptr<Device> GetDevice();
std::shared_ptr<PhysicalDevice> GetPhysicalDevice();
std::shared_ptr<SharedBufferManager> VertexAttribBufferMgr(uint32_t vertexFormat);
std::shared_ptr<SharedBufferManager> IndexBufferMgr();
std::shared_ptr<SharedBufferManager> UniformBufferMgr();
std::shared_ptr<SharedBufferManager> ShaderStorageBufferMgr();
std::shared_ptr<SharedBufferManager> IndirectBufferMgr();
std::shared_ptr<SharedBufferManager> StreamingBufferMgr();
std::shared_ptr<ThreadTaskQueue> GlobalThreadTaskQueue();
std::shared_ptr<GlobalVulkanStates> GetGlobalVulkanStates();
std::shared_ptr<PerFrameResource> MainThreadPerFrameRes();

class GlobalDeviceObjects : public Singleton<GlobalDeviceObjects>
{
public:
	bool InitObjects(const std::shared_ptr<Device>& pDevice);

	~GlobalDeviceObjects();

public:
	const std::shared_ptr<Device> GetDevice() const { return m_pDevice; }
	const std::shared_ptr<Queue> GetGraphicQueue() const { return m_pGraphicQueue; }
	const std::shared_ptr<Queue> GetComputeQueue() const { return m_pComputeQueue; }
	const std::shared_ptr<Queue> GetTransferQueue() const { return m_pTransferQueue; }
	const std::shared_ptr<Queue> GetPresentQueue() const { return m_pPresentQueue; }
	const std::shared_ptr<CommandPool> GetMainThreadGraphicCmdPool() const { return m_pMainThreadGraphicCmdPool; }
	const std::shared_ptr<CommandPool> GetMainThreadComputeCmdPool() const { return m_pMainThreadComputeCmdPool; }
	const std::shared_ptr<CommandPool> GetMainThreadTransferCmdPool() const { return m_pMainThreadTransferCmdPool; }
	const std::shared_ptr<DeviceMemoryManager> GetDeviceMemMgr() const { return m_pDeviceMemMgr; }
	const std::shared_ptr<StagingBufferManager> GetStagingBufferMgr() const { return m_pStaingBufferMgr; }
	const std::shared_ptr<SwapChain> GetSwapChain() const { return m_pSwapChain; }
	const std::shared_ptr<SharedBufferManager> GetVertexAttribBufferMgr(uint32_t vertexFormat);
	const std::shared_ptr<SharedBufferManager> GetIndexBufferMgr() const { return m_pIndexBufferMgr; }
	const std::shared_ptr<SharedBufferManager> GetUniformBufferMgr() const { return m_pUniformBufferMgr; }
	const std::shared_ptr<SharedBufferManager> GetShaderStorageBufferMgr() const { return m_pShaderStorageBufferMgr; }
	const std::shared_ptr<SharedBufferManager> GetIndirectBufferMgr() const { return m_pIndirectBufferMgr; }
	const std::shared_ptr<SharedBufferManager> GetStreamingBufferMgr() const { return m_pStreamingBufferMgr; }
	const std::shared_ptr<ThreadTaskQueue> GetThreadTaskQueue() const { return m_pThreadTaskQueue; }
	const std::shared_ptr<GlobalVulkanStates> GetGlobalVulkanStates() const { return m_pGlobalVulkanStates; }
	const std::shared_ptr<PerFrameResource> GetMainThreadPerFrameRes() const;

	//FIXME : remove me
	bool RequestAttributeBuffer(uint32_t size, uint32_t& offset);

protected:
	std::shared_ptr<Device>					m_pDevice;
	std::shared_ptr<Queue>					m_pGraphicQueue;
	std::shared_ptr<Queue>					m_pComputeQueue;
	std::shared_ptr<Queue>					m_pTransferQueue;
	std::shared_ptr<Queue>					m_pPresentQueue;
	std::shared_ptr<CommandPool>			m_pMainThreadGraphicCmdPool;
	std::shared_ptr<CommandPool>			m_pMainThreadComputeCmdPool;
	std::shared_ptr<CommandPool>			m_pMainThreadTransferCmdPool;
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

	std::vector<std::shared_ptr<PerFrameResource>> m_mainThreadPerFrameRes;

	static const uint32_t ATTRIBUTE_BUFFER_SIZE = 1024 * 1024 * 64;
	static const uint32_t INDEX_BUFFER_SIZE = 1024 * 1024 * 4;
	static const uint32_t UNIFORM_BUFFER_SIZE = 1024 * 512;
	static const uint32_t SHADER_STORAGE_BUFFER_SIZE = 1024 * 1024 * 2;
	static const uint32_t INDIRECT_BUFFER_SIZE = 1024 * 1024;

	uint32_t								m_attributeBufferOffset = 0;
};