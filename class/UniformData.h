#pragma once
#include "GlobalUniforms.h"
#include "PerFrameUniforms.h"
#include "PerObjectUniforms.h"
#include "GBufferInputUniforms.h"
#include "GlobalTextures.h"
#include "../common/Singleton.h"
#include "../Maths/Matrix.h"
#include "../Base/Base.h"

class DescriptorPool;
class DescriptorSetLayout;
class DescriptorSet;

class UniformData : public Singleton<UniformData>
{
public:
	enum UniformDataLayoutLocation
	{
		GlobalUniformsLocation,
		PerFrameUniformsLocation,
		PerObjectUniformsLocation,
		PerObjectMaterialVariableBufferLocation,
		UniformDataLayoutLocationCount
	};

	enum UniformStorageType
	{
		GlobalVariableBuffer,
		PerBoneBuffer,
		PerFrameVariableBuffer,
		PerFrameBonesBuffer,
		PerObjectVariableBuffer,
		PerObjectMaterialVariableBuffer,
		UniformStorageTypeCount
	};

	enum UniformTextureType
	{
		GlobalUniformTextures,
		GlobalGBufferInputUniforms,
		UniformTextureTypeCount
	};

public:
	bool Init() override;

public:
	std::shared_ptr<GlobalUniforms> GetGlobalUniforms() const { return std::dynamic_pointer_cast<GlobalUniforms>(m_uniformStorageBuffers[UniformStorageType::GlobalVariableBuffer]); }
	std::shared_ptr<PerBoneUniforms> GetPerBoneUniforms() const { return std::dynamic_pointer_cast<PerBoneUniforms>(m_uniformStorageBuffers[UniformStorageType::PerBoneBuffer]); }
	std::shared_ptr<PerFrameUniforms> GetPerFrameUniforms() const { return std::dynamic_pointer_cast<PerFrameUniforms>(m_uniformStorageBuffers[UniformStorageType::PerFrameVariableBuffer]); }
	std::shared_ptr<PerFrameBoneUniforms> GetPerFrameBoneUniforms() const { return std::dynamic_pointer_cast<PerFrameBoneUniforms>(m_uniformStorageBuffers[UniformStorageType::PerFrameBonesBuffer]); }
	std::shared_ptr<PerObjectUniforms> GetPerObjectUniforms() const { return std::dynamic_pointer_cast<PerObjectUniforms>(m_uniformStorageBuffers[UniformStorageType::PerObjectVariableBuffer]); }
	std::shared_ptr<UniformDataStorage> GetUniformStorage(UniformStorageType uniformStorageType) const { return m_uniformStorageBuffers[uniformStorageType]; }
	
	std::shared_ptr<GlobalTextures> GetGlobalTextures() const { return std::dynamic_pointer_cast<GlobalTextures>(m_uniformTextures[UniformTextureType::GlobalUniformTextures]); }
	std::shared_ptr<GBufferInputUniforms> GetGBufferInputUniforms() const { return std::dynamic_pointer_cast<GBufferInputUniforms>(m_uniformTextures[UniformTextureType::GlobalGBufferInputUniforms]); }
	std::shared_ptr<UniformBase> GetUniformTextures(UniformTextureType uniformTextureType) const { return m_uniformTextures[uniformTextureType]; }

	void SyncDataBuffer();
	std::vector<std::vector<UniformVarList>> GenerateUniformVarLayout() const;
	std::vector<std::vector<uint32_t>> GetCachedFrameOffsets() const;

	std::vector<std::shared_ptr<DescriptorSetLayout>> GetDescriptorSetLayouts() const { return m_descriptorSetLayouts; }
	std::vector<std::shared_ptr<DescriptorSet>> GetDescriptorSets() const { return m_descriptorSets; }

protected:
	void BuildDescriptorSets();

protected:
	std::vector<std::shared_ptr<UniformDataStorage>>	m_uniformStorageBuffers;
	std::vector<std::shared_ptr<UniformBase>>			m_uniformTextures;

	std::shared_ptr<DescriptorPool>						m_pDescriptorPool;
	std::vector<std::shared_ptr<DescriptorSetLayout>>	m_descriptorSetLayouts;
	std::vector<std::shared_ptr<DescriptorSet>>			m_descriptorSets;

	std::vector<std::vector<uint32_t>>					m_cachedFrameOffsets;
};