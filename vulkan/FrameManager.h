#pragma once

#include "DeviceObjectBase.h"
#include <map>

class CommandBuffer;
class Fence;
class PerFrameResource;

class FrameManager
{
	typedef std::map<uint32_t, std::vector<std::shared_ptr<PerFrameResource>>> FrameResourceTable;
public:
	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount);

public:
	std::shared_ptr<PerFrameResource> AllocatePerFrameResource(uint32_t frameIndex);

public:
	static std::shared_ptr<FrameManager> Create(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount);

protected:
	void IncFrameIndex() { m_currentFrameIndex = (m_currentFrameIndex + 1) % m_maxFrameCount; }
	uint32_t FrameIndex() const { return m_currentFrameIndex; }

private:
	FrameResourceTable						m_frameResTable;
	std::vector<std::shared_ptr<Fence>>		m_frameFences;

	uint32_t m_currentFrameIndex;
	uint32_t m_maxFrameCount;
};