#pragma once
#include "vulkan.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include <vector>

class Queue;

class Device
{
public:
	~Device();

	bool Init(const std::shared_ptr<Instance>& pInst, const std::shared_ptr<PhysicalDevice>& pPhyisicalDevice);

public:
	const VkDevice GetDeviceHandle() const { return m_device; }
	const std::shared_ptr<PhysicalDevice> GetPhysicalDevice() const { return m_pPhysicalDevice; }
	const std::shared_ptr<Instance> GetInstance() const { return m_pVulkanInst; }
	const std::shared_ptr<Queue> GetGraphicQueue() const { return m_pGraphicQueue; }
	const std::shared_ptr<Queue> GetPresentQueue() const { return m_pPresentQueue; }

public:
	static std::shared_ptr<Device> Create(const std::shared_ptr<Instance>& pInstance, const std::shared_ptr<PhysicalDevice> pPhyisicalDevice);

protected:
	VkDevice							m_device;
	std::shared_ptr<PhysicalDevice>		m_pPhysicalDevice;
	std::shared_ptr<Instance>			m_pVulkanInst;
	std::shared_ptr<Queue>				m_pGraphicQueue;
	std::shared_ptr<Queue>				m_pPresentQueue;
};