#include "../class/Material.h"
#include "MaterialInstance.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/FrameManager.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/SwapChain.h"
#include "../class/UniformData.h"
#include "../component/MeshRenderer.h"

MaterialInstance::~MaterialInstance()
{
}

bool MaterialInstance::Init(const std::shared_ptr<MaterialInstance>& pMaterialInstance)
{
	if (!SelfRefBase<MaterialInstance>::Init(pMaterialInstance))
		return false;

	return true;
}

void MaterialInstance::SetMaterialTexture(uint32_t parameterIndex, InGameTextureType type, const std::string& textureName)
{
	uint32_t textureIndex;
	if (!UniformData::GetInstance()->GetGlobalTextures()->GetTextureIndex(type, textureName, textureIndex))
		SetParameter(parameterIndex, (float)-1);
	else
		SetParameter(parameterIndex, (float)textureIndex);
}

void MaterialInstance::BindPipeline(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	GetMaterial()->BindPipeline(pCmdBuffer);
}

void MaterialInstance::BindDescriptorSet(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	m_pMaterial->BindDescriptorSet(pCmdBuffer);
}

void MaterialInstance::PrepareMaterial(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	BindPipeline(pCmdBuffer);
	BindDescriptorSet(pCmdBuffer);
}

void MaterialInstance::Draw()
{
	for (auto & pRenderer : m_meshRenderers)
		FrameMgr()->AddJobToFrame(std::bind(&BaseComponent::Draw, pRenderer.lock().get(), std::placeholders::_1));
}

void MaterialInstance::InsertIntoRenderQueue(const VkDrawIndexedIndirectCommand& cmd, uint32_t perObjectIndex)
{
	m_pMaterial->InsertIntoRenderQueue(cmd, perObjectIndex, m_materialBufferChunkIndex);
}