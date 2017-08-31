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

	std::vector<VkAttachmentDescription> attachmentDescs(2);
	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescs[0].format = pDevice->GetPhysicalDevice()->GetSurfaceFormat().format;
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;

	attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs[1].format = pDevice->GetPhysicalDevice()->GetDepthStencilFormat();
	attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference colorAttach = {};
	colorAttach.attachment = 0;
	colorAttach.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference dsAttach = {};
	dsAttach.attachment = 1;
	dsAttach.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttach;
	subpass.pDepthStencilAttachment = &dsAttach;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstSubpass = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderpassCreateInfo = {};
	renderpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpassCreateInfo.attachmentCount = attachmentDescs.size();
	renderpassCreateInfo.pAttachments = attachmentDescs.data();
	renderpassCreateInfo.subpassCount = 1;
	renderpassCreateInfo.pSubpasses = &subpass;
	renderpassCreateInfo.dependencyCount = 1;
	renderpassCreateInfo.pDependencies = &subpassDependency;

	m_framebuffers.resize(GlobalDeviceObjects::GetInstance()->GetSwapChain()->GetSwapChainImageCount());
	for (uint32_t i = 0; i < m_framebuffers.size(); i++)
		m_framebuffers[i] = FrameBuffer::Create(m_pDevice, m_pSwapChain->GetSwapChainImage(i), DepthStencilBuffer::Create(m_pDevice), RenderPass::Create(m_pDevice, renderpassCreateInfo));

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