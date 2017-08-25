#include "CommandBuffer.h"
#include "CommandPool.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "DescriptorSet.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "GraphicPipeline.h"
#include "PipelineLayout.h"
#include "Buffer.h"
#include "Image.h"

CommandBuffer::~CommandBuffer()
{
	vkFreeCommandBuffers(GetDevice()->GetDeviceHandle(), m_pCommandPool->GetDeviceHandle(), 1, &m_commandBuffer);
}

bool CommandBuffer::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<CommandBuffer>& pSelf, const VkCommandBufferAllocateInfo& info)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
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

	if (pCommandBuffer.get() && pCommandBuffer->Init(pDevice, pCommandBuffer, info))
		return pCommandBuffer;
	return nullptr;
}

void CommandBuffer::PrepareNormalDrawCommands(const DrawCmdData& data)
{
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CHECK_VK_ERROR(vkBeginCommandBuffer(GetDeviceHandle(), &cmdBeginInfo));

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.clearValueCount = data.clearValues.size();
	renderPassBeginInfo.pClearValues = data.clearValues.data();
	renderPassBeginInfo.renderPass = data.pRenderPass->GetDeviceHandle();
	renderPassBeginInfo.framebuffer = data.pFrameBuffer->GetDeviceHandle();
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
	for (uint32_t i = 0; i < data.descriptorSets.size(); i++)
		dsSets.push_back(data.descriptorSets[i]->GetDeviceHandle());

	vkCmdBindDescriptorSets(GetDeviceHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, data.pPipeline->GetPipelineLayout()->GetDeviceHandle(), 0, dsSets.size(), dsSets.data(), 0, nullptr);

	vkCmdBindPipeline(GetDeviceHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, data.pPipeline->GetDeviceHandle());

	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkDeviceSize> offsets;
	for (uint32_t i = 0; i < data.vertexBuffers.size(); i++)
	{
		vertexBuffers.push_back(data.vertexBuffers[i]->GetDeviceHandle());
		offsets.push_back(0);
	}
	vkCmdBindVertexBuffers(GetDeviceHandle(), 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
	vkCmdBindIndexBuffer(GetDeviceHandle(), data.pIndexBuffer->GetDeviceHandle(), 0, data.pIndexBuffer->GetType());

	vkCmdDrawIndexed(GetDeviceHandle(), data.pIndexBuffer->GetCount(), 1, 0, 0, 0);

	vkCmdEndRenderPass(GetDeviceHandle());

	CHECK_VK_ERROR(vkEndCommandBuffer(GetDeviceHandle()));

	m_drawCmdData = data;
}

void CommandBuffer::CopyBuffer(const std::shared_ptr<Buffer>& pSrc, const std::shared_ptr<Buffer>& pDst, const std::vector<VkBufferCopy>& regions)
{
	std::vector<VkBufferMemoryBarrier> bufferBarriers;

	// Src barriers
	for (uint32_t i = 0; i < regions.size(); i++)
	{
		VkBufferMemoryBarrier bufferBarrier = {};
		bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier.srcAccessMask = pSrc->GetAccessFlags();
		bufferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		bufferBarrier.buffer = pSrc->GetDeviceHandle();
		bufferBarrier.offset = regions[i].srcOffset;
		bufferBarrier.size = regions[i].size;
		bufferBarriers.push_back(bufferBarrier);
	}

	AttachBarriers
	(
		pSrc->GetAccessStages(),
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		{},
		bufferBarriers,
		{}
	);

	bufferBarriers.clear();

	// Dst barriers
	for (uint32_t i = 0; i < regions.size(); i++)
	{
		VkBufferMemoryBarrier bufferBarrier = {};
		bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier.srcAccessMask = pDst->GetAccessFlags();
		bufferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		bufferBarrier.buffer = pDst->GetDeviceHandle();
		bufferBarrier.offset = regions[i].dstOffset;
		bufferBarrier.size = regions[i].size;
		bufferBarriers.push_back(bufferBarrier);
	}

	AttachBarriers
	(
		pDst->GetAccessStages(),
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		{},
		bufferBarriers,
		{}
	);

	bufferBarriers.clear();

	vkCmdCopyBuffer(GetDeviceHandle(), pSrc->GetDeviceHandle(), pDst->GetDeviceHandle(), regions.size(), regions.data());

	// Src barriers
	for (uint32_t i = 0; i < regions.size(); i++)
	{
		VkBufferMemoryBarrier bufferBarrier = {};
		bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		bufferBarrier.dstAccessMask = pSrc->GetAccessFlags();
		bufferBarrier.buffer = pSrc->GetDeviceHandle();
		bufferBarrier.offset = regions[i].srcOffset;
		bufferBarrier.size = regions[i].size;
		bufferBarriers.push_back(bufferBarrier);
	}

	AttachBarriers
	(
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		pSrc->GetAccessStages(),
		{},
		bufferBarriers,
		{}
	);

	bufferBarriers.clear();

	// Dst barriers
	for (uint32_t i = 0; i < regions.size(); i++)
	{
		VkBufferMemoryBarrier bufferBarrier = {};
		bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		bufferBarrier.dstAccessMask = pDst->GetAccessFlags();
		bufferBarrier.buffer = pDst->GetDeviceHandle();
		bufferBarrier.offset = regions[i].dstOffset;
		bufferBarrier.size = regions[i].size;
		bufferBarriers.push_back(bufferBarrier);
	}

	AttachBarriers
	(
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		pDst->GetAccessStages(),
		{},
		bufferBarriers,
		{}
	);

	AddToReferenceTable(pSrc);
	AddToReferenceTable(pDst);
}

void CommandBuffer::CopyImage(const std::shared_ptr<Image>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkImageCopy>& regions)
{
	vkCmdCopyImage
	(
		GetDeviceHandle(), 
		pSrc->GetDeviceHandle(), pSrc->GetImageInfo().initialLayout,
		pDst->GetDeviceHandle(), pDst->GetImageInfo().initialLayout,
		regions.size(), regions.data()
	);

	AddToReferenceTable(pSrc);
	AddToReferenceTable(pDst);
}

void CommandBuffer::CopyBufferImage(const std::shared_ptr<Buffer>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkBufferImageCopy>& regions)
{

}

void CommandBuffer::PrepareBufferCopyCommands(const BufferCopyCmdData& data)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	CHECK_VK_ERROR(vkBeginCommandBuffer(GetDeviceHandle(), &beginInfo));

	std::vector<VkBufferMemoryBarrier> barriers;
	VkPipelineStageFlagBits srcStages;
	VkPipelineStageFlagBits dstStages;

	if (data.preBarriers.size() != 0)
	{
		srcStages = data.preBarriers[0].srcStages;
		dstStages = data.preBarriers[0].dstStages;
		for (uint32_t i = 0; i < data.preBarriers.size(); i++)
		{
			if (srcStages == data.preBarriers[i].srcStages
				&& dstStages == data.preBarriers[i].dstStages)
			{
				VkBufferMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				barrier.srcAccessMask = data.preBarriers[i].srcAccess;
				barrier.dstAccessMask = data.preBarriers[i].dstAccess;
				barrier.buffer = data.preBarriers[i].pBuffer->GetDeviceHandle();
				barrier.offset = data.preBarriers[i].offset;
				barrier.size = data.preBarriers[i].size;
				barriers.push_back(barrier);
			}
			else
			{
				vkCmdPipelineBarrier(GetDeviceHandle(),
					srcStages,
					dstStages,
					0,
					0, nullptr,
					barriers.size(), barriers.data(),
					0, nullptr);

				barriers.clear();
				srcStages = data.preBarriers[i].srcStages;
				dstStages = data.preBarriers[i].dstStages;
			}
		}

		vkCmdPipelineBarrier(GetDeviceHandle(),
			srcStages,
			dstStages,
			0,
			0, nullptr,
			barriers.size(), barriers.data(),
			0, nullptr);

		barriers.clear();
	}

	// Copy each chunk to dst buffer
	for (uint32_t i = 0; i < data.copyData.size(); i++)
		vkCmdCopyBuffer(GetDeviceHandle(), data.copyData[i].pSrcBuffer->GetDeviceHandle(), data.copyData[i].pDstBuffer->GetDeviceHandle(), 1, &data.copyData[i].copyData);

	if (data.postBarriers.size() != 0)
	{
		srcStages = data.postBarriers[0].srcStages;
		dstStages = data.postBarriers[0].dstStages;
		for (uint32_t i = 0; i < data.preBarriers.size(); i++)
		{
			if (srcStages == data.postBarriers[i].srcStages
				&& dstStages == data.postBarriers[i].dstStages)
			{
				VkBufferMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				barrier.srcAccessMask = data.postBarriers[i].srcAccess;
				barrier.dstAccessMask = data.postBarriers[i].dstAccess;
				barrier.buffer = data.postBarriers[i].pBuffer->GetDeviceHandle();
				barrier.offset = data.postBarriers[i].offset;
				barrier.size = data.postBarriers[i].size;
				barriers.push_back(barrier);
			}
			else
			{
				vkCmdPipelineBarrier(GetDeviceHandle(),
					srcStages,
					dstStages,
					0,
					0, nullptr,
					barriers.size(), barriers.data(),
					0, nullptr);

				barriers.clear();
				srcStages = data.preBarriers[i].srcStages;
				dstStages = data.preBarriers[i].dstStages;
			}
		}

		vkCmdPipelineBarrier(GetDeviceHandle(),
			srcStages,
			dstStages,
			0,
			0, nullptr,
			barriers.size(), barriers.data(),
			0, nullptr);

		barriers.clear();
	}

	CHECK_VK_ERROR(vkEndCommandBuffer(GetDeviceHandle()));

	m_bufferCopyCmdData = data;
}

void CommandBuffer::StartRecording()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	CHECK_VK_ERROR(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
}

void CommandBuffer::EndRecording()
{
	CHECK_VK_ERROR(vkEndCommandBuffer(m_commandBuffer));
}

void CommandBuffer::AttachBarriers
(
	VkPipelineStageFlags src,
	VkPipelineStageFlags dst,
	const std::vector<VkMemoryBarrier>& memBarriers,
	const std::vector<VkBufferMemoryBarrier>& bufferMemBarriers,
	const std::vector<VkImageMemoryBarrier>& imageMemBarriers
)
{
	vkCmdPipelineBarrier
	(
		GetDeviceHandle(),
		src,
		dst,
		0,
		memBarriers.size(), memBarriers.data(),
		bufferMemBarriers.size(), bufferMemBarriers.data(),
		imageMemBarriers.size(), imageMemBarriers.data()
	);
}