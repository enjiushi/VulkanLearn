#include "../common/RefCounted.h"
#include "vulkan.h"
#include "VulkanInstance.h"
#include "PhysicalDevice.h"
#include "VulkanInstance.h"
#include <vector>

class VulkanDevice : public RefCounted
{
public:
	static VulkanDevice* CreateVulkanDevice(const VulkanInstance* pInst, const PhysicalDevice* pPhyisicalDevice);
	~VulkanDevice();

protected:
	VulkanDevice() : m_device(0) {}
	bool Init(const VulkanInstance* pInst, const PhysicalDevice* pPhyisicalDevice);

protected:
	VkDevice							m_device;
	AutoPTR<PhysicalDevice>				m_pPhysicalDevice;
	AutoPTR<VulkanInstance>				m_pVulkanInst;

	PFN_vkCreateSwapchainKHR			m_fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR			m_fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR			m_fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR			m_fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR				m_fpQueuePresentKHR;
};