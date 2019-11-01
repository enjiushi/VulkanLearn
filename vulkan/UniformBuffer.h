#pragma once

#include "SharedBuffer.h"

class UniformBuffer : public SharedBuffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		const std::shared_ptr<UniformBuffer>& pSelf,
		uint32_t numBytes);

public:
	static std::shared_ptr<UniformBuffer> Create(const std::shared_ptr<Device>& pDevice,
		uint32_t numBytes);

public:
	VkDescriptorBufferInfo GetDescBufferInfo() const { return m_pBufferKey->GetSharedBufferMgr()->GetBufferDesc(m_pBufferKey); }

protected:
	std::shared_ptr<BufferKey>	AcquireBuffer(uint32_t numBytes) override;
};