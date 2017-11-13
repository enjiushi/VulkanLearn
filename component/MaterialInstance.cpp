#include "../class/Material.h"
#include "MaterialInstance.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/FrameManager.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/SwapChain.h"
#include "../class/UniformData.h"
#include "MeshRenderer.h"

MaterialInstance::~MaterialInstance()
{
}

bool MaterialInstance::Init(const std::shared_ptr<MaterialInstance>& pMaterialInstance)
{
	if (!SelfRefBase<MaterialInstance>::Init(pMaterialInstance))
		return false;

	return true;
}

void MaterialInstance::SetMaterialTexture(uint32_t index, const std::shared_ptr<Image>& pTexture)
{
	m_textures[index] = pTexture;
	m_pMaterial->SetMaterialTexture(index, pTexture);
}

void MaterialInstance::BindPipeline(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	pCmdBuffer->BindPipeline(GetMaterial()->GetGraphicPipeline());
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