#include "VulkanBase.h"

class PhysicalDevice
{
public:
	PhysicalDevice();
	PhysicalDevice(VkDevice device);
	~DeviceMemoryManager();

public:
	bool AllocateMemory(uint32_t byteSize);

protected:
	void ReleaseMemory();

protected:
	VkPhysicalDevice					m_physicalDevice;
	VkPhysicalDeviceProperties			m_physicalDeviceProperties;
	VkPhysicalDeviceFeatures			m_physicalDeviceFeatures;
	VkPhysicalDeviceMemoryProperties	m_physicalDeviceMemoryProperties;
};