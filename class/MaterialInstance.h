#pragma once
#include "../Base/BaseComponent.h"
#include "GlobalTextures.h"

class DescriptorSet;
class DescriptorPool;
class Material;
class Image;
class CommandBuffer;
class MeshRenderer;

class MaterialInstance : public SelfRefBase<MaterialInstance>
{
public:
	~MaterialInstance();

	std::shared_ptr<Material> GetMaterial() const { return m_pMaterial; }
	uint32_t GetRenderMask() const { return m_renderMask; }
	void SetRenderMask(uint32_t renderMask) { m_renderMask = renderMask; }
	void SetMaterialTexture(uint32_t parameterIndex, InGameTextureType type, const std::string& textureName);
	void SetMaterialTexture(const std::string& paramName, InGameTextureType type, const std::string& textureName);
	void PrepareMaterial(const std::shared_ptr<CommandBuffer>& pCmdBuffer);
	void Draw();

	// FIXME: should add name based functions to ease of use
	template <typename T>
	void SetParameter(uint32_t parameterIndex, T val)
	{
		m_pMaterial->SetParameter(m_materialBufferChunkIndex, parameterIndex, val);
	}

	template <typename T>
	T GetParameter(uint32_t parameterIndex)
	{
		return m_pMaterial->GetParameter<T>(m_materialBufferChunkIndex, parameterIndex);
	}

	template <typename T>
	void SetParameter(const std::string& paramName, T val)
	{
		m_pMaterial->SetParameter(m_materialBufferChunkIndex, paramName, val);
	}

	template <typename T>
	T GetParameter(const std::string& paramName)
	{
		return m_pMaterial->GetParameter<T>(m_materialBufferChunkIndex, paramName);
	}

	void InsertIntoRenderQueue(const std::shared_ptr<Mesh>& pMesh, uint32_t perObjectIndex, uint32_t perMeshIndex, uint32_t perAnimationIndex, uint32_t instanceCount, uint32_t instanceDataOffset);

protected:
	bool Init(const std::shared_ptr<MaterialInstance>& pMaterialInstance);
	void BindPipeline(const std::shared_ptr<CommandBuffer>& pCmdBuffer);
	void BindDescriptorSet(const std::shared_ptr<CommandBuffer>& pCmdBuffer);
	uint32_t AddMeshRenderer(const std::shared_ptr<MeshRenderer>& pRenderer) { m_meshRenderers.push_back(pRenderer); return m_meshRenderers.size() - 1; }
	void DelMeshRenderer(uint32_t index) { m_meshRenderers.erase(m_meshRenderers.begin() + index); }

protected:
	std::shared_ptr<Material>					m_pMaterial;
	std::vector<uint32_t>						m_materialVariables;
	uint32_t									m_renderMask = 0xffffffff;
	uint32_t									m_materialBufferChunkIndex;
	std::vector<std::weak_ptr<MeshRenderer>>	m_meshRenderers;

	friend class Material;
	friend class MeshRenderer;
};