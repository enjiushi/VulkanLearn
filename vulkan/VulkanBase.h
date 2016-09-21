#include "vulkan.h"

class VulkanBase
{
public:
	VulkanBase() : m_device(0) {}
	VulkanBase(VkDevice device) : m_device(device) {}
	virtual ~VulkanBase() {}

protected:
	VkDevice m_device;
};