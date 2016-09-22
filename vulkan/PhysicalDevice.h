#include "../common/RefCounted.h"
#include "vulkan.h"
#include <vector>

class VulkanInstance;

class PhysicalDevice : public RefCounted
{
public:
	static PhysicalDevice* AcquirePhysicalDevice(const VulkanInstance* pVulkanInstance);
	~PhysicalDevice();

	const VkPhysicalDevice GetDeviceHandle() const { return m_physicalDevice; }
	const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const { return m_physicalDeviceProperties; }
	const VkPhysicalDeviceFeatures& GetPhysicalDeviceFeatures() const { return m_physicalDeviceFeatures; }
	const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const {
		return m_physicalDeviceMemoryProperties;
	}

	const std::vector<VkQueueFamilyProperties>& GetQueueProperties() const { return m_queueProperties; }
	const VkFormat GetDepthStencilFormat() const { return m_depthStencilFormat; }

protected:
	PhysicalDevice() : m_physicalDevice(0) {}
	bool Init(const VulkanInstance* pVulkanInstance);

protected:
	VkPhysicalDevice					m_physicalDevice;
	VkPhysicalDeviceProperties			m_physicalDeviceProperties;
	VkPhysicalDeviceFeatures			m_physicalDeviceFeatures;
	VkPhysicalDeviceMemoryProperties	m_physicalDeviceMemoryProperties;

	std::vector<VkQueueFamilyProperties>	m_queueProperties;
	VkFormat							m_depthStencilFormat;
};