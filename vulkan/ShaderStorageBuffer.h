#pragma once

#include "Buffer.h"
#include "GlobalDeviceObjects.h"
#include "SharedBufferManager.h"

class ShaderStorageBuffer : public Buffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		const std::shared_ptr<ShaderStorageBuffer>& pSelf,
		uint32_t numBytes);

public:
	VkDescriptorBufferInfo GetDescBufferInfo() const { return m_pBufferKey->GetSharedBufferMgr()->GetBufferDesc(m_pBufferKey); }
	VkBuffer GetDeviceHandle() const override { return m_pBufferKey->GetSharedBufferMgr()->GetBuffer()->GetDeviceHandle(); }
	void UpdateByteStream(const void* pData, uint32_t offset, uint32_t numBytes) override;
	uint32_t GetBufferOffset() const override;

public:
	static std::shared_ptr<ShaderStorageBuffer> Create(const std::shared_ptr<Device>& pDevice,
		uint32_t numBytes);

protected:
	std::shared_ptr<BufferKey> m_pBufferKey;
};