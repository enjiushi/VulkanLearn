#pragma once

#include "DeviceObjectBase.h"

class CommandPool;
class GraphicPipeline;
class RenderPass;
class DescriptorSet;
class VertexBuffer;
class IndexBuffer;
class FrameBuffer;

class CommandBuffer : public DeviceObjectBase
{
public:
	typedef struct _CmdBufData
	{
		std::shared_ptr<RenderPass>					pRenderPass;
		std::shared_ptr<FrameBuffer>				pFrameBuffer;
		std::shared_ptr<GraphicPipeline>			pPipeline;
		std::vector<std::shared_ptr<DescriptorSet>>	descriptorSets;

		std::vector<std::shared_ptr<VertexBuffer>>	vertexBuffers;
		std::shared_ptr<IndexBuffer>				pIndexBuffer;

		std::vector<VkClearValue>					clearValues;
	}CmdBufData;

public:
	~CommandBuffer();

	bool Init(const std::shared_ptr<Device>& pDevice, const VkCommandBufferAllocateInfo& info);

public:
	VkCommandBuffer GetDeviceHandle() const { return m_commandBuffer; }
	VkCommandBufferAllocateInfo GetAllocateInfo() const { return m_info; }
	bool DrawCommandsReady() const { return m_drawCommandsReady; }
	void SetCmdBufData(const CmdBufData& data);
	void PrepareNormalDrawCommands();

public:
	static std::shared_ptr<CommandBuffer> Create(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<CommandPool>& pCmdPool, VkCommandBufferLevel cmdBufferLevel);

protected:
	VkCommandBuffer					m_commandBuffer;
	VkCommandBufferAllocateInfo		m_info;

	std::shared_ptr<CommandPool>	m_pCommandPool;

	CmdBufData						m_cmdBufData;
	bool							m_drawCommandsReady = false;
};