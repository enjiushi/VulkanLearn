#pragma once

#include "../common/Singleton.h"
#include "Device.h"

class Queue;
class CommandPool;
class DeviceMemoryManager;
class StagingBufferManager;

class GlobalDeviceObjects : public Singleton<GlobalDeviceObjects>
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice);

public:
	const std::shared_ptr<Queue> GetGraphicQueue() const { return m_pGraphicQueue; }
	const std::shared_ptr<Queue> GetPresentQueue() const { return m_pPresentQueue; }
	const std::shared_ptr<CommandPool> GetMainThreadCmdPool() const { return m_pMainThreadCmdPool; }
	const std::shared_ptr<DeviceMemoryManager> GetDeviceMemMgr() const { return m_pDeviceMemMgr; }
	const std::shared_ptr<StagingBufferManager> GetStagingBufferMgr() const { return m_pStaingBufferMgr; }

protected:
	std::shared_ptr<Queue>					m_pGraphicQueue;
	std::shared_ptr<Queue>					m_pPresentQueue;
	std::shared_ptr<CommandPool>			m_pMainThreadCmdPool;
	std::shared_ptr<DeviceMemoryManager>	m_pDeviceMemMgr;
	std::shared_ptr<StagingBufferManager>	m_pStaingBufferMgr;
};