#pragma once
#include "GlobalUniforms.h"
#include "PerFrameUniforms.h"
#include "PerObjectUniforms.h"
#include "../common/Singleton.h"
#include "../Maths/Matrix.h"
#include "../Base/Base.h"

class DescriptorSet;

class UniformData : public Singleton<UniformData>
{
public:
	bool Init() override;

public:
	std::shared_ptr<GlobalUniforms> GetGlobalUniforms() const { return std::dynamic_pointer_cast<GlobalUniforms>(m_uniforms[UniformDataStorage::GlobalVariable]); }
	std::shared_ptr<PerFrameUniforms> GetPerFrameUniforms() const { return std::dynamic_pointer_cast<PerFrameUniforms>(m_uniforms[UniformDataStorage::PerFrameVariable]); }
	std::shared_ptr<PerObjectUniforms> GetPerObjectUniforms() const { return std::dynamic_pointer_cast<PerObjectUniforms>(m_uniforms[UniformDataStorage::PerObjectVariable]); }
	std::shared_ptr<UniformDataStorage> GetUniformStorage(UniformDataStorage::UniformType uniformType) const { return m_uniforms[uniformType]; }
	
	void SyncDataBuffer();
	std::vector<std::vector<UniformVarList>> GenerateUniformVarLayout() const;
	std::vector<uint32_t> GetFrameOffsets() const;

	void SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, UniformDataStorage::UniformType uniformType, uint32_t reservedIndex = 0) const;

protected:
	std::vector<std::shared_ptr<UniformDataStorage>>	m_uniforms;
};