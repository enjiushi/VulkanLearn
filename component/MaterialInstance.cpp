#include "../class/Material.h"
#include "MaterialInstance.h"
#include "../vulkan/DescriptorSet.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/FrameManager.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/SwapChain.h"
#include "../class/UniformData.h"

bool MaterialInstance::Init(const std::shared_ptr<MaterialInstance>& pMaterialInstance)
{
	if (!BaseComponent::Init(pMaterialInstance))
		return false;

	return true;
}

void MaterialInstance::SetMaterialTexture(uint32_t index, const std::shared_ptr<Image>& pTexture)
{
	m_textures[index - 1] = pTexture;

	// index 0 is reserved for material uniform buffer FIXME: there should a enum or something to mark it
	m_descriptorSets[UniformDataStorage::PerObjectMaterialVariable]->UpdateImage(index, pTexture);
}

void MaterialInstance::BindPipeline(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	pCmdBuffer->BindPipeline(GetMaterial()->GetGraphicPipeline());
}

void MaterialInstance::BindDescriptorSet(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	std::vector<uint32_t> offsets = UniformData::GetInstance()->GetFrameOffsets();
	std::vector<uint32_t> materialOffsets = m_pMaterial->GetFrameOffsets();
	offsets.insert(offsets.end(), materialOffsets.begin(), materialOffsets.end());
	pCmdBuffer->BindDescriptorSets(GetMaterial()->GetPipelineLayout(), GetDescriptorSets(), offsets);
}

void MaterialInstance::PrepareMaterial(const std::shared_ptr<CommandBuffer>& pCmdBuffer)
{
	BindPipeline(pCmdBuffer);
	BindDescriptorSet(pCmdBuffer);
}