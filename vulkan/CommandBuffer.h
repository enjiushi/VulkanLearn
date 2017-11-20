#pragma once

#include "DeviceObjectBase.h"

class CommandPool;
class GraphicPipeline;
class RenderPass;
class DescriptorSet;
class VertexBuffer;
class IndexBuffer;
class FrameBuffer;
class Buffer;
class Image;
class PipelineLayout;
class IndirectBuffer;

class CommandBuffer : public DeviceObjectBase<CommandBuffer>
{
public:
	typedef struct _DrawCmdData
	{
		std::shared_ptr<RenderPass>					pRenderPass;
		std::shared_ptr<FrameBuffer>				pFrameBuffer;
		std::shared_ptr<GraphicPipeline>			pPipeline;
		std::vector<std::shared_ptr<DescriptorSet>>	descriptorSets;

		std::vector<std::shared_ptr<VertexBuffer>>	vertexBuffers;
		std::shared_ptr<IndexBuffer>				pIndexBuffer;

		std::vector<VkClearValue>					clearValues;
	}DrawCmdData;

	typedef struct _BarrierData
	{
		std::shared_ptr<Buffer>						pBuffer;
		VkPipelineStageFlagBits						srcStages;
		VkPipelineStageFlagBits						dstStages;
		VkAccessFlags								srcAccess;
		VkAccessFlags								dstAccess;
		uint32_t									offset;
		uint32_t									size;
	}BarrierData;

	typedef struct _CopyData
	{
		std::shared_ptr<Buffer>						pSrcBuffer;
		VkBufferCopy								copyData;
		std::shared_ptr<Buffer>						pDstBuffer;
	}CopyData;

	typedef struct _BufferCopyCmdData
	{
		std::vector<BarrierData>					preBarriers;
		std::vector<CopyData>						copyData;
		std::vector<BarrierData>					postBarriers;
	}BufferCopyCmdData;

public:
	~CommandBuffer();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<CommandBuffer>& pSelf, const VkCommandBufferAllocateInfo& info);

public:
	VkCommandBuffer GetDeviceHandle() const { return m_commandBuffer; }
	VkCommandBufferAllocateInfo GetAllocateInfo() const { return m_info; }
	std::shared_ptr<CommandPool> GetCommandPool() const { return m_pCommandPool; }

	void StartPrimaryRecording();
	void EndPrimaryRecording();
	void StartSecondaryRecording(const VkCommandBufferInheritanceInfo& inheritanceInfo);
	void EndSecondaryRecording();
	void ExecuteSecondaryCommandBuffer(const std::vector<std::shared_ptr<CommandBuffer>>& cmdBuffers);
	void PrepareNormalDrawCommands(const DrawCmdData& data);
	void PrepareBufferCopyCommands(const BufferCopyCmdData& data);

	void CopyBuffer(const std::shared_ptr<Buffer>& pSrc, const std::shared_ptr<Buffer>& pDst, const std::vector<VkBufferCopy>& regions);
	void CopyImage(const std::shared_ptr<Image>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkImageCopy>& regions);
	void CopyBufferImage(const std::shared_ptr<Buffer>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkBufferImageCopy>& regions);

	void AttachBarriers
	(
		VkPipelineStageFlags src,
		VkPipelineStageFlags dst,
		const std::vector<VkMemoryBarrier>& memBarriers,
		const std::vector<VkBufferMemoryBarrier>& bufferMemBarriers,
		const std::vector<VkImageMemoryBarrier>& imageMemBarriers
	);

	void SetViewports(const std::vector<VkViewport>& viewports);
	void SetScissors(const std::vector<VkRect2D>& scissors);

	void BindDescriptorSets(const std::shared_ptr<PipelineLayout>& pPipelineLayout, const std::vector<std::shared_ptr<DescriptorSet>>& descriptorSets, const std::vector<uint32_t>& offsets);
	void BindPipeline(const std::shared_ptr<GraphicPipeline>& pPipeline);
	void BindVertexBuffers(const std::vector<std::shared_ptr<Buffer>>& vertexBuffers);
	void BindIndexBuffer(const std::shared_ptr<Buffer>& pIndexBuffer, VkIndexType type);

	void BeginRenderPass(const std::shared_ptr<FrameBuffer>& pFrameBuffer, const std::shared_ptr<RenderPass>& pRenderPass, const std::vector<VkClearValue>& clearValues, bool includeSecondary = false);
	void EndRenderPass();

	void DrawIndexed(const std::shared_ptr<IndexBuffer>& pIndexBuffer);
	void DrawIndexedIndirect(const std::shared_ptr<IndirectBuffer>& pIndirectBuffer, uint32_t offset, uint32_t count);

protected:
	static std::shared_ptr<CommandBuffer> Create(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<CommandPool>& pCmdPool, VkCommandBufferLevel cmdBufferLevel);

protected:
	bool IsValide() const { m_isValide; }
	void SetIsValide(bool flag) { m_isValide = flag; }

	void IssueBarriersBeforeCopy(const std::shared_ptr<Buffer>& pSrc, const std::shared_ptr<Buffer>& pDst, const std::vector<VkBufferCopy>& regions);
	void IssueBarriersAfterCopy(const std::shared_ptr<Buffer>& pSrc, const std::shared_ptr<Buffer>& pDst, const std::vector<VkBufferCopy>& regions);

	void IssueBarriersBeforeCopy(const std::shared_ptr<Buffer>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkBufferImageCopy>& regions);
	void IssueBarriersAfterCopy(const std::shared_ptr<Buffer>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkBufferImageCopy>& regions);

	void IssueBarriersBeforeCopy(const std::shared_ptr<Image>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkImageCopy>& regions);
	void IssueBarriersAfterCopy(const std::shared_ptr<Image>& pSrc, const std::shared_ptr<Image>& pDst, const std::vector<VkImageCopy>& regions);

protected:
	VkCommandBuffer									m_commandBuffer;
	VkCommandBufferAllocateInfo						m_info;
	bool											m_isValide;

	std::shared_ptr<CommandPool>					m_pCommandPool;

	DrawCmdData										m_drawCmdData;
	BufferCopyCmdData								m_bufferCopyCmdData;

	friend class CommandPool;
};