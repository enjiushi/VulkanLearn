#pragma once
#include "DeviceObjectBase.h"
#include "VKGPUSyncRes.h"

class BufferBase : public VKGPUSyncRes
{
public:
	virtual ~BufferBase() = default;

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<BufferBase>& pSelf, const VkBufferCreateInfo& info);

public:
	VkPipelineStageFlags GetAccessStages() const { return m_accessStages; }
	VkAccessFlags GetAccessFlags() const { return m_accessFlags; }
	const VkBufferCreateInfo& GetBufferInfo() const { return m_info; }

	void PrepareBarriers
	(
		VkAccessFlags				srcAccessFlags,
		VkImageLayout				srcImageLayout,
		VkAccessFlags				dstAccessFlags,
		VkImageLayout				dstImageLayout,
		std::vector<VkMemoryBarrier>&		memBarriers,
		std::vector<VkBufferMemoryBarrier>&	bufferMemBarriers,
		std::vector<VkImageMemoryBarrier>&	imageMemBarriers
	) override;

	virtual uint32_t GetBufferOffset() const = 0;
	virtual bool IsHostVisible() const = 0;
	virtual VkBuffer GetDeviceHandle() const = 0;
	virtual void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes) = 0;

protected:
	VkBufferCreateInfo				m_info;
	VkPipelineStageFlags			m_accessStages;
	VkAccessFlags					m_accessFlags;
};