#include "UniformData.h"
#include "Material.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/FrameManager.h"

bool UniformData::Init()
{
	if (!Singleton<UniformData>::Init())
		return false;

	m_uniforms.resize(UniformDataStorage::PerObjectMaterialVariable);

	for (uint32_t i = 0; i < UniformDataStorage::PerObjectMaterialVariable; i++)
	{
		switch ((UniformDataStorage::UniformType)i)
		{
		case UniformDataStorage::GlobalVariable: m_uniforms[i] = GlobalUniforms::Create(); break;
		case UniformDataStorage::PerFrameVariable: m_uniforms[i] = PerFrameUniforms::Create(); break;
		case UniformDataStorage::PerObjectVariable: m_uniforms[i] = PerObjectUniforms::Create(); break;
		default:
			break;
		}
	}

	return true;
}

void UniformData::SyncDataBuffer()
{
	for (auto& var : m_uniforms)
		var->SyncBufferData();
}

std::vector<UniformVarList> UniformData::GenerateUniformVarLayout() const
{
	std::vector<UniformVarList> layout;
	for (uint32_t i = 0; i < UniformDataStorage::PerObjectMaterialVariable; i++)
		layout.push_back(m_uniforms[i]->PrepareUniformVarList());
	return layout;
}

std::vector<uint32_t> UniformData::GetFrameOffsets() const
{
	std::vector<uint32_t> offsets;
	for (uint32_t i = 0; i < UniformDataStorage::PerObjectMaterialVariable; i++)
		offsets.push_back(m_uniforms[i]->GetFrameOffset() * FrameMgr()->FrameIndex());
	return offsets;
}
