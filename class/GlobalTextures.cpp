#include "../vulkan/Texture2DArray.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "GlobalTextures.h"
#include "Material.h"

bool GlobalTextures::Init(const std::shared_ptr<GlobalTextures>& pSelf)
{
	if (!SelfRefBase<GlobalTextures>::Init(pSelf))
		return false;

	InitTextureDiction();

	return true;
}

// FIXME: Make it configurable future
void GlobalTextures::InitTextureDiction()
{
	m_textureDiction.resize(InGameTextureTypeCount);

	m_textureDiction[RGBA8_1024].textureArrayName = "RGBA8TextureArray";
	m_textureDiction[RGBA8_1024].textureArrayDescription = "RGBA8, size16, mipLevel11";
	m_textureDiction[RGBA8_1024].pTextureArray = Texture2DArray::CreateEmptyTexture2DArray(GetDevice(), 1024, 1024, 11, 16, VK_FORMAT_R8G8B8A8_UNORM);
	m_textureDiction[RGBA8_1024].maxSlotIndex = 0;
	m_textureDiction[RGBA8_1024].currentEmptySlot = 0;

	m_textureDiction[R8_1024].textureArrayName = "R8TextureArray";
	m_textureDiction[R8_1024].textureArrayDescription = "R8, size16, mipLevel11";
	m_textureDiction[R8_1024].pTextureArray = Texture2DArray::CreateEmptyTexture2DArray(GetDevice(), 1024, 1024, 11, 16, VK_FORMAT_R8_UNORM);
	m_textureDiction[R8_1024].maxSlotIndex = 0;
	m_textureDiction[R8_1024].currentEmptySlot = 0;
}

std::shared_ptr<GlobalTextures> GlobalTextures::Create()
{
	std::shared_ptr<GlobalTextures> pGlobalTextures = std::make_shared<GlobalTextures>();
	if (pGlobalTextures.get() && pGlobalTextures->Init(pGlobalTextures))
		return pGlobalTextures;
	return nullptr;
}

std::vector<UniformVarList> GlobalTextures::PrepareUniformVarList()
{
	return 
	{
		{
			CombinedSampler,
			"RGBA8_1024_Texture_Array"
		},
		{
			CombinedSampler,
			"R8_1024_Texture_Array"
		}
	};
}

void GlobalTextures::InsertTexture(InGameTextureType type, const TextureDesc& desc, const gli::texture2d& gliTexture2d)
{
	uint32_t emptySlot = m_textureDiction[type].currentEmptySlot;
	m_textureDiction[type].textureDescriptions[emptySlot] = desc;
	m_textureDiction[type].pTextureArray->InsertTexture(gliTexture2d, emptySlot);

	// Find if there's an available slot within the pool
	bool found = false;
	for (uint32_t i = 0; i < m_textureDiction[type].maxSlotIndex; i++)
	{
		if (m_textureDiction[type].textureDescriptions.find(i) == m_textureDiction[type].textureDescriptions.end())
		{
			m_textureDiction[type].currentEmptySlot = i;
			found = true;
			break;
		}
	}

	// If there's no available slot within, then increase slot count by 1 and assign empty slot to it
	if (!found)
	{
		m_textureDiction[type].currentEmptySlot = m_textureDiction[type].maxSlotIndex + 1;
		m_textureDiction[type].maxSlotIndex = m_textureDiction[type].currentEmptySlot;
	}
}

