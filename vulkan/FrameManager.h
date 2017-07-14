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
	std::shared_ptr<PerFrameResource> AllocatePerFrameResource(uint32_t frameIndex);
	uint32_t FrameIndex() const { return m_currentFrameIndex; }

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount);
	static std::shared_ptr<FrameManager> Create(const std::shared_ptr<Device>& pDevice, uint32_t maxFrameCount);

	void SetFrameIndex(uint32_t index) { m_currentFrameIndex = index % m_maxFrameCount; }

private:
	FrameResourceTable						m_frameResTable;
	std::vector<std::shared_ptr<Fence>>		m_frameFences;

	uint32_t m_currentFrameIndex;
	uint32_t m_maxFrameCount;

	friend class SwapChain;
};