#include "CommandBuffer.h"
#include "CommandPool.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "DescriptorSet.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "GraphicPipeline.h"
#include "PipelineLayout.h"

CommandBuffer::~CommandBuffer()
{
	vkFreeCommandBuffers(GetDevice()->GetDeviceHandle(), m_pCommandPool->GetDeviceHandle(), 1, &m_commandBuffer);
}

bool CommandBuffer::Init(const std::shared_ptr<Device>& pDevice, const VkCommandBufferAllocateInfo& info)
{
	if (!DeviceObjectBase::Init(pDevice))
		return false;

	m_info = info;
	CHECK_VK_ERROR(vkAllocateCommandBuffers(GetDevice()->GetDeviceHandle(), &m_info, &m_commandBuffer));

	return true;
}

std::shared_ptr<CommandBuffer> CommandBuffer::Create(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<CommandPool>& pCmdPool, VkCommandBufferLevel cmdBufferLevel)
{
	std::shared_ptr<CommandBuffer> pCommandBuffer = std::make_shared<CommandBuffer>();
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = pCmdPool->GetDeviceHandle();
	info.commandBufferCount = 1;
	info.commandPool = pCmdPool->GetDeviceHandle();
	pCommandBuffer->m_pCommandPool = pCmdPool;

	if (pCommandBuffer.get() && pCommandBuffer->Init(pDevice, info))
		return pCommandBuffer;
	return nullptr;
}

void CommandBuffer::SetCmdBufData(const CmdBufData& data)
{
	m_cmdBufData = data;
	m_drawCommandsReady = false;

	PrepareNormalDrawCommands();
}
void CommandBuffer::PrepareNormalDrawCommands()
{
	if (m_drawCommandsReady)
		return;

	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CHECK_VK_ERROR(vkBeginCommandBuffer(GetDeviceHandle(), &cmdBeginInfo));

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.clearValueCount = m_cmdBufData.clearValues.size();
	renderPassBeginInfo.pClearValues = m_cmdBufData.clearValues.data();
	renderPassBeginInfo.renderPass = m_cmdBufData.pRenderPass->GetDeviceHandle();
	renderPassBeginInfo.framebuffer = m_cmdBufData.pFrameBuffer->GetDeviceHandle();
	renderPassBeginInfo.renderArea.extent.width = GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
	renderPassBeginInfo.renderArea.extent.height = GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;

	vkCmdBeginRenderPass(GetDeviceHandle(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport =
	{
		0, 0,
		GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height,
		0, 1
	};

	VkRect2D scissorRect =
	{
		0, 0,
		GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height
	};

	vkCmdSetViewport(GetDeviceHandle(), 0, 1, &viewport);
	vkCmdSetScissor(GetDeviceHandle(), 0, 1, &scissorRect);

	std::vector<VkDescriptorSet> dsSets;
	for (uint32_t i = 0; i < m_cmdBufData.descriptorSets.size(); i++)
		dsSets.push_back(m_cmdBufData.descriptorSets[i]->GetDeviceHandle());

	vkCmdBindDescriptorSets(GetDeviceHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_cmdBufData.pPipeline->GetPipelineLayout()->GetDeviceHandle(), 0, dsSets.size(), dsSets.data(), 0, nullptr);

	vkCmdBindPipeline(GetDeviceHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_cmdBufData.pPipeline->GetDeviceHandle());

	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkDeviceSize> offsets;
	for (uint32_t i = 0; i < m_cmdBufData.vertexBuffers.size(); i++)
	{
		vertexBuffers.push_back(m_cmdBufData.vertexBuffers[i]->GetDeviceHandle());
		offsets.push_back(0);
	}
	vkCmdBindVertexBuffers(GetDeviceHandle(), 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
	vkCmdBindIndexBuffer(GetDeviceHandle(), m_cmdBufData.pIndexBuffer->GetDeviceHandle(), 0, m_cmdBufData.pIndexBuffer->GetType());

	vkCmdDrawIndexed(GetDeviceHandle(), m_cmdBufData.pIndexBuffer->GetCount(), 1, 0, 0, 0);

	vkCmdEndRenderPass(GetDeviceHandle());

	CHECK_VK_ERROR(vkEndCommandBuffer(GetDeviceHandle()));

	m_drawCommandsReady = true;
}