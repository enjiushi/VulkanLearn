#pragma once

#include "Buffer.h"

class IndirectBuffer : public Buffer
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<IndirectBuffer>& pSelf, uint32_t numBytes);

public:
	static std::shared_ptr<IndirectBuffer> Create(const std::shared_ptr<Device>& pDevice, uint32_t numBytes);

public:
	void SetIndirectCmd(uint32_t index, const VkDrawIndirectCommand& cmd);

protected:
	std::vector<VkDrawIndirectCommand>	m_commands;
};