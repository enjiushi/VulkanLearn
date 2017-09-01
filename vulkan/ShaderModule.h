#pragma once

#include "DeviceObjectBase.h"

class ShaderModule : public DeviceObjectBase<ShaderModule>
{
public:
	enum ShaderType
	{
		ShaderTypeVertex,
		ShaderTypeTessellationControl,
		ShaderTypeTessellationEvaluation,
		ShaderTypeGeometry,
		ShaderTypeFragment,
		ShaderTypeCount
	};

public:
	~ShaderModule();

public:
	VkShaderModule GetDeviceHandle() const { return m_shaderModule; }
	std::wstring GetShaderPath() const { return m_shaderPath; }
	ShaderType GetShaderType() const { return m_shaderType; }

public:
	static std::shared_ptr<ShaderModule> Create(const std::shared_ptr<Device>& pDevice, const std::wstring& path, ShaderType type);

protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<ShaderModule>& pSelf, const std::wstring& path, ShaderType type);

protected:
	VkShaderModule			m_shaderModule;
	std::wstring			m_shaderPath;
	ShaderType				m_shaderType;
};