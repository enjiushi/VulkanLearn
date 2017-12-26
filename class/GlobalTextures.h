#pragma once

#include "UniformDataStorage.h"
#include <gli\gli.hpp>

class Texture2DArray;

enum InGameTextureType
{
	RGBA8_1024,
	R8_1024,
	InGameTextureTypeCount
};

typedef struct _TextureDesc
{
	std::string textureName;
	std::string texturePath;
	std::string textureDescription;
}TextureDesc;

typedef struct _TextureArrayDesc
{
	std::string						textureArrayName;
	std::string						textureArrayDescription;
	std::map<uint32_t, TextureDesc>	textureDescriptions;	// Key: index in texture array
	uint32_t						maxSlotIndex;
	uint32_t						currentEmptySlot;		// Empty slot is available for new texture, and is updated everytime when textureDescriptions changed
	std::shared_ptr<Texture2DArray> pTextureArray;
}TextureArrayDesc;

class GlobalTextures : public SelfRefBase<GlobalTextures>
{
public:
	static std::shared_ptr<GlobalTextures> Create();

public:
	std::vector<UniformVarList> PrepareUniformVarList();
	void InsertTexture(InGameTextureType type, const TextureDesc& desc, const gli::texture2d& gliTexture2d);
	std::shared_ptr<Texture2DArray>	GetTextureArray(InGameTextureType type) { return m_textureDiction[type].pTextureArray; }

protected:
	bool Init(const std::shared_ptr<GlobalTextures>& pSelf);
	void InitTextureDiction();

protected:
	std::vector<TextureArrayDesc>	m_textureDiction;
};