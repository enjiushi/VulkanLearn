#include "GlobalDeviceObjects.h"
#include "Queue.h"
#include "CommandPool.h"
#include "DeviceMemoryManager.h"
#include "StagingBufferManager.h"
#include "SwapChain.h"
#include "../thread/ThreadWorker.hpp"
#include "SharedBufferManager.h"
#include "Framebuffer.h"
#include "DepthStencilBuffer.h"
#include "RenderPass.h"
#include "../thread/ThreadTaskQueue.hpp"
#include "RenderWorkManager.h"

bool GlobalDeviceObjects::Init(const std::shared_ptr<Device>& pDevice)
{
	m_pDevice = pDevice;

	m_pGraphicQueue = Queue::Create(pDevice, pDevice->GetPhysicalDevice()->GetGraphicQueueIndex());
	m_pPresentQueue = Queue::Create(pDevice, pDevice->GetPhysicalDevice()->GetPresentQueueIndex());

	m_pMainThreadCmdPool = CommandPool::Create(pDevice);

	m_pDeviceMemMgr = DeviceMemoryManager::Create(pDevice);

	m_pStaingBufferMgr = StagingBufferManager::Create(pDevice);

	m_pVertexAttribBufferMgr = SharedBufferManager::Create(pDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, ATTRIBUTE_BUFFER_SIZE);
	m_pIndexBufferMgr = SharedBufferManager::Create(pDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, INDEX_BUFFER_SIZE);
	m_pUniformBufferMgr = SharedBufferManager::Create(pDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, UNIFORM_BUFFER_SIZE);

	m_pSwapChain = SwapChain::Create(pDevice);

	m_pRenderWorkMgr = RenderWorkManager::Create(pDevice);

	m_framebuffers.resize(GlobalDeviceObjects::GetInstance()->GetSwapChain()->GetSwapChainImageCount());
	for (uint32_t i = 0; i < m_framebuffers.size(); i++)
		m_framebuffers[i] = FrameBuffer::Create(m_pDevice, m_pSwapChain->GetSwapChainImage(i), DepthStencilBuffer::Create(m_pDevice), RenderWorkManager::GetDefaultRenderPass());

	m_pThreadTaskQueue = std::make_shared<ThreadTaskQueue>(pDevice, FrameMgr()->MaxFrameCount(), FrameMgr());

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

const std::shared_ptr<FrameBuffer> GlobalDeviceObjects::GetCurrentFrameBuffer() const
{ 
	return m_framebuffers[FrameMgr()->FrameIndex()]; 
}

std::shared_ptr<Queue> GlobalGraphicQueue() { return GlobalObjects()->GetGraphicQueue(); }
std::shared_ptr<Queue> GlobalPresentQueue() { return GlobalObjects()->GetPresentQueue(); }
std::shared_ptr<CommandPool> MainThreadPool() { return GlobalObjects()->GetMainThreadCmdPool(); }
std::shared_ptr<DeviceMemoryManager> DeviceMemMgr() { return GlobalObjects()->GetDeviceMemMgr(); }
std::shared_ptr<StagingBufferManager> StagingBufferMgr() { return GlobalObjects()->GetStagingBufferMgr(); }
std::shared_ptr<SwapChain> GetSwapChain() { return GlobalObjects()->GetSwapChain(); }
std::shared_ptr<FrameManager> FrameMgr();
std::shared_ptr<FrameManager> FrameMgr() { return GetSwapChain()->GetFrameManager(); }
std::shared_ptr<Device> GetDevice() { return GlobalObjects()->GetDevice(); }
std::shared_ptr<PhysicalDevice> GetPhysicalDevice() { return GetDevice()->GetPhysicalDevice(); }
std::shared_ptr<SharedBufferManager> VertexAttribBufferMgr() { return GlobalObjects()->GetVertexAttribBufferMgr(); }
std::shared_ptr<SharedBufferManager> IndexBufferMgr() { return GlobalObjects()->GetIndexBufferMgr(); }
std::shared_ptr<SharedBufferManager> UniformBufferMgr() { return GlobalObjects()->GetUniformBufferMgr(); }
std::shared_ptr<ThreadTaskQueue> GlobalThreadTaskQueue() { return GlobalObjects()->GetThreadTaskQueue(); }
std::vector<std::shared_ptr<FrameBuffer>> DefaultFrameBuffers() { return GlobalObjects()->GetDefaultFrameBuffers(); }
std::shared_ptr<RenderWorkManager> RenderWorkMgr() { return GlobalObjects()->GetRenderWorkMgr(); }