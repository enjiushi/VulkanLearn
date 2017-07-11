#pragma once

#include "DeviceObjectBase.h"

class ShaderModule : public DeviceObjectBase<ShaderModule>
{
public:
	~ShaderModule();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<ShaderModule>& pSelf, const std::wstring& path);

public:
	VkShaderModule GetDeviceHandle() const { return m_shaderModule; }
	std::wstring GetShaderPath() const { return m_shaderPath; }

public:
	static std::shared_ptr<ShaderModule> Create(const std::shared_ptr<Device>& pDevice, const std::wstring& path);

protected:
	VkShaderModule			m_shaderModule;
	std::wstring			m_shaderPath;
};