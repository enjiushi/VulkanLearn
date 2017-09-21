#include "MaterialInstance.h"
#include "Material.h"
#include "../vulkan/DescriptorSet.h"

bool MaterialInstance::Init(const std::shared_ptr<MaterialInstance>& pMaterialInstance)
{
	if (!BaseComponent::Init(pMaterialInstance))
		return false;

	return true;
}

void MaterialInstance::SetMaterialTexture(uint32_t index, const std::shared_ptr<Image>& pTexture)
{
	m_textures[index] = pTexture;

	// index 0 is reserved for material uniform buffer FIXME: there should a enum or something to mark it
	m_descriptorSets[PerObjectMaterialVariable]->UpdateImage(index + 1, pTexture);
}