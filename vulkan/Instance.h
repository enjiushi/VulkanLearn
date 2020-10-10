#pragma once
#include <memory>
#include "vulkan.h"
#include "../common/Macros.h"

class Instance
{
public:
	~Instance();

	VkInstance GetDeviceHandle() { return m_vulkanInst; }
	const VkInstance GetDeviceHandle() const { return m_vulkanInst; }

	bool Init(const VkInstanceCreateInfo&);

public:
	static std::shared_ptr<Instance> Create(const VkInstanceCreateInfo&);

private:
	VkInstance	m_vulkanInst;

#ifdef _DEBUG
	PFN_vkCreateDebugReportCallbackEXT				m_fpCreateDebugReportCallbackEXT;
	PFN_vkDebugReportMessageEXT						m_fpDebugReportMessageEXT;
	PFN_vkDestroyDebugReportCallbackEXT				m_fpDestroyDebugReportCallbackEXT;
	VkDebugReportCallbackEXT						m_debugCallback;
#endif
};