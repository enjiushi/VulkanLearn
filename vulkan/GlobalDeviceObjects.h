#pragma once

#include "../common/Singleton.h"
#include "Device.h"

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
class RenderWorkManager;
class GlobalVulkanStates;

class GlobalDeviceObjects;

GlobalDeviceObjects* GlobalObjects();
std::shared_ptr<Queue> GlobalGraphicQueue();
std::shared_ptr<Queue> GlobalPresentQueue();
std::shared_ptr<CommandPool> MainThreadPool();
std::shared_ptr<DeviceMemoryManager> DeviceMemMgr();
std::shared_ptr<StagingBufferManager> StagingBufferMgr();
std::shared_ptr<SwapChain> GetSwapChain();
std::shared_ptr<FrameManager> FrameMgr();
std::shared_ptr<Device> GetDevice();
std::shared_ptr<PhysicalDevice> GetPhysicalDevice();
std::shared_ptr<SharedBufferManager> VertexAttribBufferMgr();
std::shared_ptr<SharedBufferManager> IndexBufferMgr();
std::shared_ptr<SharedBufferManager> UniformBufferMgr();
std::shared_ptr<ThreadTaskQueue> GlobalThreadTaskQueue();
std::vector<std::shared_ptr<FrameBuffer>> DefaultFrameBuffers();
std::shared_ptr<RenderWorkManager> RenderWorkMgr();
std::shared_ptr<GlobalVulkanStates> GetGlobalVulkanStates();

class GlobalDeviceObjects : public Singleton<GlobalDeviceObjects>
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice);

	~GlobalDeviceObjects();

public:
	const std::shared_ptr<Device> GetDevice() const { return m_pDevice; }
	const std::shared_ptr<Queue> GetGraphicQueue() const { return m_pGraphicQueue; }
	const std::shared_ptr<Queue> GetPresentQueue() const { return m_pPresentQueue; }
	const std::shared_ptr<CommandPool> GetMainThreadCmdPool() const { return m_pMainThreadCmdPool; }
	const std::shared_ptr<DeviceMemoryManager> GetDeviceMemMgr() const { return m_pDeviceMemMgr; }
	const std::shared_ptr<StagingBufferManager> GetStagingBufferMgr() const { return m_pStaingBufferMgr; }
	const std::shared_ptr<SwapChain> GetSwapChain() const { return m_pSwapChain; }
	const std::shared_ptr<SharedBufferManager> GetVertexAttribBufferMgr() const { return m_pVertexAttribBufferMgr; }
	const std::shared_ptr<SharedBufferManager> GetIndexBufferMgr() const { return m_pIndexBufferMgr; }
	const std::shared_ptr<SharedBufferManager> GetUniformBufferMgr() const { return m_pUniformBufferMgr; }
	const std::shared_ptr<FrameBuffer> GetCurrentFrameBuffer() const;
	const std::vector<std::shared_ptr<FrameBuffer>> GetDefaultFrameBuffers() const { return m_framebuffers; }
	const std::shared_ptr<ThreadTaskQueue> GetThreadTaskQueue() const { return m_pThreadTaskQueue; }
	const std::shared_ptr<RenderWorkManager> GetRenderWorkMgr() const { return m_pRenderWorkMgr; }
	const std::shared_ptr<GlobalVulkanStates> GetGlobalVulkanStates() const { return m_pGlobalVulkanStates; }

	//FIXME : remove me
	bool RequestAttributeBuffer(uint32_t size, uint32_t& offset);

protected:
	std::shared_ptr<Device>					m_pDevice;
	std::shared_ptr<Queue>					m_pGraphicQueue;
	std::shared_ptr<Queue>					m_pPresentQueue;
	std::shared_ptr<CommandPool>			m_pMainThreadCmdPool;
	std::shared_ptr<DeviceMemoryManager>	m_pDeviceMemMgr;
	std::shared_ptr<StagingBufferManager>	m_pStaingBufferMgr;
	std::shared_ptr<SwapChain>				m_pSwapChain;
	std::shared_ptr<SharedBufferManager>	m_pVertexAttribBufferMgr;
	std::shared_ptr<SharedBufferManager>	m_pIndexBufferMgr;
	std::shared_ptr<SharedBufferManager>	m_pUniformBufferMgr;
	std::shared_ptr<RenderWorkManager>		m_pRenderWorkMgr;
	std::shared_ptr<GlobalVulkanStates>		m_pGlobalVulkanStates;

	std::shared_ptr<ThreadTaskQueue>		m_pThreadTaskQueue;

	std::vector<std::shared_ptr<FrameBuffer>>m_framebuffers;

	static const uint32_t UNIFORM_BUFFER_SIZE = 1024 * 1024 * 8;
	static const uint32_t ATTRIBUTE_BUFFER_SIZE = 1024 * 1024 * 256;
	static const uint32_t INDEX_BUFFER_SIZE = 1024 * 1024 * 32;

	uint32_t								m_attributeBufferOffset = 0;
};