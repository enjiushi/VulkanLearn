#include "CommandBuffer.h"
#include "CommandPool.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "DescriptorSet.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "PipelineBase.h"
#include "PipelineLayout.h"
#include "Buffer.h"
#include "Image.h"
#include "VulkanUtil.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "../vulkan/GlobalVulkanStates.h"
#include "GlobalDeviceObjects.h"
#include "IndirectBuffer.h"
#include "../common/Enums.h"

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
	info.level = cmdBufferLevel;
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
	renderPassBeginInfo.clearValueCount = (uint32_t)data.clearValues.size();
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
		(float)GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width, (float)GetDevice()->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height,
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

	vkCmdBindDescriptorSets(GetDeviceHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, data.pPipeline->GetPipelineLayout()->GetDeviceHandle(), 0, (uint32_t)dsSets.size(), dsSets.data(), 0, nullptr);

	vkCmdBindPipeline(GetDeviceHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, data.pPipeline->GetDeviceHandle());

	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkDeviceSize> offsets;
	for (uint32_t i = 0; i < data.vertexBuffers.size(); i++)
	{
		vertexBuffers.push_back(data.vertexBuffers[i]->GetDeviceHandle());
		offsets.push_back(0);
	}
	vkCmdBindVertexBuffers(GetDeviceHandle(), 0, (uint32_t)vertexBuffers.size(), vertexBuffers.data(), offsets.data());
	vkCmdBindIndexBuffer(GetDeviceHandle(), data.pIndexBuffer->GetDeviceHandle(), 0, data.pIndexBuffer->GetType());

	vkCmdDrawIndexed(GetDeviceHandle(), data.pIndexBuffer->GetCount(), 1, 0, 0, 0);

	vkCmdEndRenderPass(GetDeviceHandle());

	CHECK_VK_ERROR(vkEndCommandBuffer(GetDeviceHandle()));

	m_drawCmdData = data;
}

void CommandBuffer::IssueBarriersBeforeCopy(const std::shared_ptr<BufferBase>& pSrc, const std::shared_ptr<BufferBase>& pDst, const std::vector<VkBufferCopy>& regions)
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
}

void CommandBuffer::IssueBarriersAfterCopy(const std::shared_ptr<BufferBase>& pSrc, const std::shared_ptr<BufferBase>& pDst, const std::vector<VkBufferCopy>& regions)
{
	std::vector<VkBufferMemoryBarrier> bufferBarriers;

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
}

void CommandBuffer::IssueBarriersBeforeCopy(const std::shared_ptr<BufferBase>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkBufferImageCopy>& regions) 
{
	std::vector<VkBufferMemoryBarrier> bufferBarriers;
	
	for (uint32_t i = 0; i < regions.size(); i++)
	{
		VkBufferMemoryBarrier bufferBarrier = {};
		bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier.srcAccessMask = pSrc->GetAccessFlags();
		bufferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		bufferBarrier.buffer = pSrc->GetDeviceHandle();
		bufferBarrier.offset = regions[i].bufferOffset;

		if (i < regions.size() - 1)
			bufferBarrier.size = regions[i + 1].bufferOffset - regions[i].bufferOffset;
		else
			bufferBarrier.size = pSrc->GetBufferInfo().size - regions[i].bufferOffset;

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

	std::vector<VkImageMemoryBarrier> imgBarriers;

	for (uint32_t i = 0; i < regions.size(); i++)
	{
		//Barrier for layout change from undefined to transfer dst
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = regions[i].imageSubresource.aspectMask;
		subresourceRange.baseMipLevel = regions[i].imageSubresource.mipLevel;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = regions[i].imageSubresource.baseArrayLayer;
		subresourceRange.layerCount = regions[i].imageSubresource.layerCount;

		VkImageMemoryBarrier imgBarrier = {};
		imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imgBarrier.image = pDst->GetDeviceHandle();
		imgBarrier.subresourceRange = subresourceRange;
		imgBarrier.oldLayout = pDst->GetImageInfo().initialLayout;
		imgBarrier.srcAccessMask = pDst->GetAccessFlags();
		imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		imgBarriers.push_back(imgBarrier);
	}

	AttachBarriers
	(
		pDst->GetAccessStages(),
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		{},
		{},
		imgBarriers
	);
}
void CommandBuffer::IssueBarriersAfterCopy(const std::shared_ptr<BufferBase>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkBufferImageCopy>& regions)
{
	std::vector<VkBufferMemoryBarrier> bufferBarriers;

	for (uint32_t i = 0; i < regions.size(); i++)
	{
		VkBufferMemoryBarrier bufferBarrier = {};
		bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		bufferBarrier.dstAccessMask = pSrc->GetAccessFlags();
		bufferBarrier.buffer = pSrc->GetDeviceHandle();
		bufferBarrier.offset = regions[i].bufferOffset;

		if (i < regions.size() - 1)
			bufferBarrier.size = regions[i + 1].bufferOffset - regions[i].bufferOffset;
		else
			bufferBarrier.size = pSrc->GetBufferInfo().size - regions[i].bufferOffset;

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

	std::vector<VkImageMemoryBarrier> imgBarriers;

	for (uint32_t i = 0; i < regions.size(); i++)
	{
		//Barrier for layout change from undefined to transfer dst
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = regions[i].imageSubresource.aspectMask;
		subresourceRange.baseMipLevel = regions[i].imageSubresource.mipLevel;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = regions[i].imageSubresource.baseArrayLayer;
		subresourceRange.layerCount = regions[i].imageSubresource.layerCount;

		VkImageMemoryBarrier imgBarrier = {};
		imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imgBarrier.image = pDst->GetDeviceHandle();
		imgBarrier.subresourceRange = subresourceRange;
		imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imgBarrier.newLayout = pDst->GetImageInfo().initialLayout;
		imgBarrier.dstAccessMask = pDst->GetAccessFlags();

		imgBarriers.push_back(imgBarrier);
	}

	AttachBarriers
	(
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		pDst->GetAccessStages(),
		{},
		{},
		imgBarriers
	);
}

void CommandBuffer::IssueBarriersBeforeCopy(const std::shared_ptr<Image>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkImageCopy>& regions) 
{
	std::vector<VkImageMemoryBarrier> imgBarriers;

	for (uint32_t i = 0; i < regions.size(); i++)
	{
		VkImageMemoryBarrier imgBarrier = {};
		imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imgBarrier.image = pSrc->GetDeviceHandle();
		imgBarrier.oldLayout = pSrc->GetImageInfo().initialLayout;
		imgBarrier.srcAccessMask = pSrc->GetAccessFlags();
		imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imgBarrier.subresourceRange =
		{
			regions[i].srcSubresource.aspectMask,
			regions[i].srcSubresource.mipLevel, 1,
			regions[i].srcSubresource.baseArrayLayer, regions[i].srcSubresource.layerCount
		};

		imgBarriers.push_back(imgBarrier);
	}

	AttachBarriers
	(
		pSrc->GetAccessStages(),
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		{},
		{},
		imgBarriers
	);

	imgBarriers.clear();

	for (uint32_t i = 0; i < regions.size(); i++)
	{
		VkImageMemoryBarrier imgBarrier = {};
		imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imgBarrier.image = pDst->GetDeviceHandle();
		imgBarrier.oldLayout = pDst->GetImageInfo().initialLayout;
		imgBarrier.srcAccessMask = pDst->GetAccessFlags();
		imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imgBarrier.subresourceRange =
		{
			regions[i].dstSubresource.aspectMask,
			regions[i].dstSubresource.mipLevel, 1,
			regions[i].dstSubresource.baseArrayLayer, regions[i].dstSubresource.layerCount
		};

		imgBarriers.push_back(imgBarrier);
	}

	AttachBarriers
	(
		pDst->GetAccessStages(),
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		{},
		{},
		imgBarriers
	);
}

void CommandBuffer::IssueBarriersAfterCopy(const std::shared_ptr<Image>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkImageCopy>& regions) 
{
	std::vector<VkImageMemoryBarrier> imgBarriers;

	for (uint32_t i = 0; i < regions.size(); i++)
	{
		VkImageMemoryBarrier imgBarrier = {};
		imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imgBarrier.image = pSrc->GetDeviceHandle();
		imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imgBarrier.newLayout = pSrc->GetImageInfo().initialLayout;
		imgBarrier.dstAccessMask = pSrc->GetAccessFlags();
		imgBarrier.subresourceRange =
		{
			regions[i].srcSubresource.aspectMask,
			regions[i].srcSubresource.mipLevel, 1,
			regions[i].srcSubresource.baseArrayLayer, regions[i].srcSubresource.layerCount
		};

		imgBarriers.push_back(imgBarrier);
	}

	AttachBarriers
	(
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		pSrc->GetAccessStages(),
		{},
		{},
		imgBarriers
	);

	imgBarriers.clear();

	for (uint32_t i = 0; i < regions.size(); i++)
	{
		VkImageMemoryBarrier imgBarrier = {};
		imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imgBarrier.image = pDst->GetDeviceHandle();
		imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imgBarrier.newLayout = pDst->GetImageInfo().initialLayout;
		imgBarrier.dstAccessMask = pDst->GetAccessFlags();
		imgBarrier.subresourceRange =
		{
			regions[i].dstSubresource.aspectMask,
			regions[i].dstSubresource.mipLevel, 1,
			regions[i].dstSubresource.baseArrayLayer, regions[i].dstSubresource.layerCount
		};

		imgBarriers.push_back(imgBarrier);
	}

	AttachBarriers
	(
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		pDst->GetAccessStages(),
		{},
		{},
		imgBarriers
	);
}

void CommandBuffer::IssueBarriersBeforeCopy(const std::shared_ptr<Image>& pSrc, const std::shared_ptr<Image>& pDst, const VkImageSubresourceLayers& srcLayers, const VkImageSubresourceLayers& dstLayers)
{
	VkImageMemoryBarrier imgBarrier = {};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pSrc->GetDeviceHandle();
	imgBarrier.oldLayout = pSrc->GetImageInfo().initialLayout;
	imgBarrier.srcAccessMask = pSrc->GetAccessFlags();
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	imgBarrier.subresourceRange =
	{
		srcLayers.aspectMask,
		srcLayers.mipLevel, 1,
		srcLayers.baseArrayLayer, srcLayers.layerCount
	};

	AttachBarriers
	(
		pSrc->GetAccessStages(),
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		{},
		{},
		{ imgBarrier }
	);

	imgBarrier = {};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pDst->GetDeviceHandle();
	imgBarrier.oldLayout = pDst->GetImageInfo().initialLayout;
	imgBarrier.srcAccessMask = pDst->GetAccessFlags();
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imgBarrier.subresourceRange =
	{
		dstLayers.aspectMask,
		dstLayers.mipLevel, 1,
		dstLayers.baseArrayLayer, dstLayers.layerCount
	};

	AttachBarriers
	(
		pDst->GetAccessStages(),
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		{},
		{},
		{ imgBarrier }
	);
}

void CommandBuffer::IssueBarriersAfterCopy(const std::shared_ptr<Image>& pSrc, const std::shared_ptr<Image>& pDst, const VkImageSubresourceLayers& srcLayers, const VkImageSubresourceLayers& dstLayers, VkPipelineStageFlags extraDstStages)
{
	VkImageMemoryBarrier imgBarrier = {};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pSrc->GetDeviceHandle();
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	imgBarrier.newLayout = pSrc->GetImageInfo().initialLayout;
	imgBarrier.dstAccessMask = pSrc->GetAccessFlags();
	imgBarrier.subresourceRange =
	{
		srcLayers.aspectMask,
		srcLayers.mipLevel, 1,
		srcLayers.baseArrayLayer, srcLayers.layerCount
	};

	AttachBarriers
	(
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		pSrc->GetAccessStages(),
		{},
		{},
		{ imgBarrier }
	);

	imgBarrier = {};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = pDst->GetDeviceHandle();
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imgBarrier.newLayout = pDst->GetImageInfo().initialLayout;
	imgBarrier.dstAccessMask = pDst->GetAccessFlags();
	imgBarrier.subresourceRange =
	{
		dstLayers.aspectMask,
		dstLayers.mipLevel, 1,
		dstLayers.baseArrayLayer, dstLayers.layerCount
	};

	AttachBarriers
	(
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		pDst->GetAccessStages() | extraDstStages,
		{},
		{},
		{ imgBarrier }
	);
}

void CommandBuffer::CopyBuffer(const std::shared_ptr<BufferBase>& pSrc, const std::shared_ptr<BufferBase>& pDst, const std::vector<VkBufferCopy>& regions)
{
	IssueBarriersBeforeCopy(pSrc, pDst, regions);

	vkCmdCopyBuffer(GetDeviceHandle(), pSrc->GetDeviceHandle(), pDst->GetDeviceHandle(), (uint32_t)regions.size(), regions.data());

	IssueBarriersAfterCopy(pSrc, pDst, regions);

	AddToReferenceTable(pSrc);
	AddToReferenceTable(pDst);
}

void CommandBuffer::BlitImage(const std::shared_ptr<Image>& pSrc, const std::shared_ptr<Image>& pDst, const VkImageBlit& blit)
{
	IssueBarriersBeforeCopy(pSrc, pDst, blit.srcSubresource, blit.dstSubresource);

	vkCmdBlitImage
	(
		GetDeviceHandle(), 
		pSrc->GetDeviceHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
		pDst->GetDeviceHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		1, &blit, 
		VK_FILTER_LINEAR
	);

	IssueBarriersAfterCopy(pSrc, pDst, blit.srcSubresource, blit.dstSubresource);

	AddToReferenceTable(pSrc);
	AddToReferenceTable(pDst);
}

void CommandBuffer::GenerateMipmaps(const std::shared_ptr<Image>& pImg, uint32_t layer)
{
	VkImageCreateInfo info = pImg->GetImageInfo();

	for (uint32_t mip = 1; mip < info.mipLevels; mip++)
	{
		VkImageBlit blit = 
		{
			{
				VK_IMAGE_ASPECT_COLOR_BIT,
				mip - 1,
				layer,
				1
			},
			{ { 0, 0, 0 }, { (int32_t)info.extent.width >> (mip - 1), (int32_t)info.extent.height >> (mip - 1), (int32_t)info.extent.depth } },

			{
				VK_IMAGE_ASPECT_COLOR_BIT,
				mip,
				layer,
				1
			},
			{ { 0, 0, 0 },{ (int32_t)info.extent.width >> mip, (int32_t)info.extent.height >> mip, (int32_t)info.extent.depth } }
		};
		BlitImage(pImg, pImg, blit);
	}
}

void CommandBuffer::CopyImage(const std::shared_ptr<Image>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkImageCopy>& regions)
{
	IssueBarriersBeforeCopy(pSrc, pDst, regions);

	vkCmdCopyImage
	(
		GetDeviceHandle(), 
		pSrc->GetDeviceHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		pDst->GetDeviceHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		(uint32_t)regions.size(), regions.data()
	);

	IssueBarriersAfterCopy(pSrc, pDst, regions);

	AddToReferenceTable(pSrc);
	AddToReferenceTable(pDst);
}

void CommandBuffer::CopyBufferImage(const std::shared_ptr<Buffer>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkBufferImageCopy>& regions)
{
	IssueBarriersBeforeCopy(pSrc, pDst, regions);

	vkCmdCopyBufferToImage(GetDeviceHandle(),
		pSrc->GetDeviceHandle(),
		pDst->GetDeviceHandle(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		(uint32_t)regions.size(),
		regions.data());

	IssueBarriersAfterCopy(pSrc, pDst, regions);

	AddToReferenceTable(pSrc);
	AddToReferenceTable(pDst);
}

void CommandBuffer::PushConstants(const std::shared_ptr<PipelineLayout>& pPipelineLayout, VkShaderStageFlags shaderFlag, uint32_t offset, uint32_t size, const void* pData)
{
	vkCmdPushConstants(GetDeviceHandle(), pPipelineLayout->GetDeviceHandle(), shaderFlag, offset, size, pData);
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
					(uint32_t)barriers.size(), barriers.data(),
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
			(uint32_t)barriers.size(), barriers.data(),
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
					(uint32_t)barriers.size(), barriers.data(),
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
			(uint32_t)barriers.size(), barriers.data(),
			0, nullptr);

		barriers.clear();
	}

	CHECK_VK_ERROR(vkEndCommandBuffer(GetDeviceHandle()));

	m_bufferCopyCmdData = data;
}

void CommandBuffer::StartPrimaryRecording()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = m_pCommandPool->GetInfo().flags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	CHECK_VK_ERROR(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
}

void CommandBuffer::EndPrimaryRecording()
{
	CHECK_VK_ERROR(vkEndCommandBuffer(m_commandBuffer));
}

void CommandBuffer::StartSecondaryRecording(const std::shared_ptr<RenderPass>& pRenderPass, uint32_t subpassIndex, const std::shared_ptr<FrameBuffer>& pFrameBuffer)
{
	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = pRenderPass->GetDeviceHandle();
	inheritanceInfo.subpass = subpassIndex;
	inheritanceInfo.framebuffer = pFrameBuffer->GetDeviceHandle();

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | (m_pCommandPool->GetInfo().flags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
	beginInfo.pInheritanceInfo = &inheritanceInfo;
	CHECK_VK_ERROR(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
}

void CommandBuffer::EndSecondaryRecording()
{
	CHECK_VK_ERROR(vkEndCommandBuffer(m_commandBuffer));
}

void CommandBuffer::ExecuteSecondaryCommandBuffer(const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers)
{
	std::vector<VkCommandBuffer> rawCmdBuffers;
	std::for_each(cmdBuffers.begin(), cmdBuffers.end(), [&rawCmdBuffers](auto& pCmdBuffer) {rawCmdBuffers.push_back(pCmdBuffer->GetDeviceHandle());});
	vkCmdExecuteCommands(GetDeviceHandle(), (uint32_t)rawCmdBuffers.size(), rawCmdBuffers.data());

	for (uint32_t i = 0; i < cmdBuffers.size(); i++)
		AddToReferenceTable(cmdBuffers[i]);
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
		(uint32_t)memBarriers.size(), memBarriers.data(),
		(uint32_t)bufferMemBarriers.size(), bufferMemBarriers.data(),
		(uint32_t)imageMemBarriers.size(), imageMemBarriers.data()
	);
}

void CommandBuffer::SetViewports(const std::vector<VkViewport>& viewports)
{
	vkCmdSetViewport(GetDeviceHandle(), 0, (uint32_t)viewports.size(), viewports.data());
}

void CommandBuffer::SetScissors(const std::vector<VkRect2D>& scissors)
{
	vkCmdSetScissor(GetDeviceHandle(), 0, (uint32_t)scissors.size(), scissors.data());
}

void CommandBuffer::BindDescriptorSets(const std::shared_ptr<PipelineLayout>& pPipelineLayout, const std::vector<std::shared_ptr<DescriptorSet>>& descriptorSets, const std::vector<uint32_t>& offsets)
{
	std::vector<VkDescriptorSet> rawDSList;
	for (uint32_t i = 0; i < (uint32_t)descriptorSets.size(); i++)
	{
		AddToReferenceTable(descriptorSets[i]);
		rawDSList.push_back(descriptorSets[i]->GetDeviceHandle());
	}

	vkCmdBindDescriptorSets
	(
		GetDeviceHandle(),
		VK_PIPELINE_BIND_POINT_GRAPHICS, pPipelineLayout->GetDeviceHandle(),
		0, (uint32_t)descriptorSets.size(), rawDSList.data(),
		(uint32_t)offsets.size(), offsets.data()
	);

	AddToReferenceTable(pPipelineLayout);
}

void CommandBuffer::BindPipeline(const std::shared_ptr<PipelineBase>& pPipeline)
{
	vkCmdBindPipeline(GetDeviceHandle(), pPipeline->GetPipelineBindingPoint(), pPipeline->GetDeviceHandle());
	AddToReferenceTable(pPipeline);
}

void CommandBuffer::BindVertexBuffer(const std::shared_ptr<BufferBase>& pBuffer, uint32_t offset, uint32_t startSlot)
{
	VkBuffer rawBuffer = pBuffer->GetDeviceHandle();
	VkDeviceSize _offset = pBuffer->GetBufferOffset() + offset;
	vkCmdBindVertexBuffers(GetDeviceHandle(), startSlot, 1, &rawBuffer, &_offset);
	AddToReferenceTable(pBuffer);
}

void CommandBuffer::BindVertexBuffers(const std::vector<std::shared_ptr<BufferBase>>& vertexBuffers, uint32_t startSlot)
{
	for (uint32_t i = 0; i < vertexBuffers.size(); i++)
	{
		BindVertexBuffer(vertexBuffers[i], 0, startSlot);
	}
}

void CommandBuffer::BindIndexBuffer(const std::shared_ptr<BufferBase>& pIndexBuffer, VkIndexType type)
{
	vkCmdBindIndexBuffer(GetDeviceHandle(), pIndexBuffer->GetDeviceHandle(), pIndexBuffer->GetBufferOffset(), type);
	AddToReferenceTable(pIndexBuffer);
}

void CommandBuffer::DrawIndexed(const std::shared_ptr<IndexBuffer>& pIndexBuffer)
{
	vkCmdDrawIndexed(GetDeviceHandle(), pIndexBuffer->GetCount(), 1, 0, 0, 0);
}

void CommandBuffer::DrawIndexed(uint32_t count)
{
	vkCmdDrawIndexed(GetDeviceHandle(), count, 1, 0, 0, 0);
}

void CommandBuffer::DrawIndexedIndirect(const std::shared_ptr<BufferBase>& pIndirectBuffer, uint32_t offset, uint32_t count)
{
	// NOTE: offset of vkCmdDrawIndexedIndirect is mesured by bytes, not elements!
	vkCmdDrawIndexedIndirect(GetDeviceHandle(), pIndirectBuffer->GetDeviceHandle(), pIndirectBuffer->GetBufferOffset() + offset * sizeof(VkDrawIndexedIndirectCommand), count, sizeof(VkDrawIndexedIndirectCommand));
	AddToReferenceTable(pIndirectBuffer);
}

void CommandBuffer::DrawIndexedIndirectCount(const std::shared_ptr<BufferBase>& pIndirectBuffer, uint32_t indirectOffset, const std::shared_ptr<BufferBase>& pIndirectCmdCountBuffer, uint32_t indirectCountOffset)
{
	(*GetDevice()->CmdDrawIndexedIndirectCountKHR())(GetDeviceHandle(), 
		pIndirectBuffer->GetDeviceHandle(), 
		pIndirectBuffer->GetBufferOffset() + indirectOffset * sizeof(VkDrawIndexedIndirectCommand), 
		pIndirectCmdCountBuffer->GetDeviceHandle(),
		pIndirectCmdCountBuffer->GetBufferOffset() + indirectCountOffset * sizeof(uint32_t),
		MAX_INDIRECT_DRAW_COUNT,
		sizeof(VkDrawIndexedIndirectCommand));
	AddToReferenceTable(pIndirectBuffer);
	AddToReferenceTable(pIndirectCmdCountBuffer);
}

void CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	vkCmdDraw(GetDeviceHandle(), vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::NextSubpass()
{
	vkCmdNextSubpass(GetDeviceHandle(), VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void CommandBuffer::Execute(const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers)
{
	std::vector<VkCommandBuffer> cmds;
	for (auto& cmd : cmdBuffers)
	{
		cmds.push_back(cmd->GetDeviceHandle());
		AddToReferenceTable(cmd);
	}

	vkCmdExecuteCommands(GetDeviceHandle(), (uint32_t)cmds.size(), cmds.data());
}

void CommandBuffer::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	vkCmdDispatch(GetDeviceHandle(), groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::BeginRenderPass(const std::shared_ptr<FrameBuffer>& pFrameBuffer, const std::shared_ptr<RenderPass>& pRenderPass, const std::vector<VkClearValue>& clearValues, bool includeSecondary)
{
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.data();
	renderPassBeginInfo.renderPass = pRenderPass->GetDeviceHandle();
	renderPassBeginInfo.framebuffer = pFrameBuffer->GetDeviceHandle();
	renderPassBeginInfo.renderArea.extent.width = pFrameBuffer->GetFramebufferInfo().width;
	renderPassBeginInfo.renderArea.extent.height = pFrameBuffer->GetFramebufferInfo().height;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;

	VkSubpassContents contents = includeSecondary ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : VK_SUBPASS_CONTENTS_INLINE;
	vkCmdBeginRenderPass(GetDeviceHandle(), &renderPassBeginInfo, contents);
	AddToReferenceTable(pFrameBuffer);
	AddToReferenceTable(pRenderPass);
}

void CommandBuffer::EndRenderPass()
{
	vkCmdEndRenderPass(GetDeviceHandle());
}