#pragma once
#include "BufferBase.h"
#include "SharedBufferManager.h"

class SharedBuffer : public BufferBase
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SharedBuffer>& pSelf, const VkBufferCreateInfo& info);

public:
	uint32_t GetBufferOffset() const override;
	bool IsHostVisible() const override;
	VkBuffer GetDeviceHandle() const override;
	void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes) override;

protected:
	virtual std::shared_ptr<BufferKey>	AcquireBuffer(uint32_t numBytes) = 0;

protected:
	std::shared_ptr<BufferKey>	m_pBufferKey;
};