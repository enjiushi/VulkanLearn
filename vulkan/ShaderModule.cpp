#include "ShaderModule.h"
#include <fstream>

ShaderModule::~ShaderModule()
{
	vkDestroyShaderModule(GetDevice()->GetDeviceHandle(), m_shaderModule, nullptr);
}

bool ShaderModule::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<ShaderModule>& pSelf, const std::wstring& path, ShaderType type, const std::string& entryName)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	std::ifstream ifs;
	ifs.open(path, std::ios::binary);
	if (ifs.fail())
		return false;

	std::vector<char> buffer;
	buffer.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = buffer.size();
	info.pCode = (uint32_t*)buffer.data();
	CHECK_VK_ERROR(vkCreateShaderModule(pDevice->GetDeviceHandle(), &info, nullptr, &m_shaderModule));

	m_shaderType = type;

	switch (m_shaderType)
	{
	case ShaderTypeVertex: m_shaderStage = VK_SHADER_STAGE_VERTEX_BIT; break;
	case ShaderTypeTessellationControl: m_shaderStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
	case ShaderTypeTessellationEvaluation: m_shaderStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; break;
	case ShaderTypeGeometry: m_shaderStage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
	case ShaderTypeFragment: m_shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
	case ShaderTypeCompute: m_shaderStage = VK_SHADER_STAGE_COMPUTE_BIT; break;
	default: ASSERTION(false);
	}

	m_entryName = entryName;

	return true;
}

std::shared_ptr<ShaderModule> ShaderModule::Create(const std::shared_ptr<Device>& pDevice, const std::wstring& path, ShaderType type, const std::string& entryName)
{
	std::shared_ptr<ShaderModule> pModule = std::make_shared<ShaderModule>();
	if (pModule.get() && pModule->Init(pDevice, pModule, path, type, entryName))
		return pModule;
	return nullptr;
}