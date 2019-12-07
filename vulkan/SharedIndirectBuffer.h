#pragma once

#include "SharedBuffer.h"

class SharedIndirectBuffer : public SharedBuffer
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SharedIndirectBuffer>& pSelf, uint32_t numBytes);

public:
	static std::shared_ptr<SharedIndirectBuffer> Create(const std::shared_ptr<Device>& pDevice, uint32_t numBytes);

public:
	void SetIndirectCmd(uint32_t index, const VkDrawIndexedIndirectCommand& cmd);
	void SetIndirectCmdCount(uint32_t count);

protected:
	std::shared_ptr<BufferKey>	AcquireBuffer(uint32_t numBytes) override;
};