#pragma once

#include "UniformDataStorage.h"
#include <gli\gli.hpp>

class Texture2D;
class TextureCube;
class Texture2DArray;

enum InGameTextureType
{
	RGBA8_1024,
	R8_1024,
	InGameTextureTypeCount
};

enum IBLTextureType
{
	RGBA16_1024_SkyBox,
	RGBA16_512_SkyBoxIrradiance,
	RGBA16_512_SkyBoxPrefilterEnv,
	IBLCubeTextureTypeCount,
	RGBA16_512_BRDFLut = 0,
	IBL2DTextureTypeCount,
	IBLTextureTypeCount = IBLCubeTextureTypeCount + IBL2DTextureTypeCount
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
	std::map<std::string, uint32_t> lookupTable;
}TextureArrayDesc;

class GlobalTextures : public SelfRefBase<GlobalTextures>
{
public:
	const static uint32_t OFFSCREEN_SIZE = 512;

public:
	static std::shared_ptr<GlobalTextures> Create();

public:
	std::vector<UniformVarList> PrepareUniformVarList();
	void InsertTexture(InGameTextureType type, const TextureDesc& desc, const gli::texture2d& gliTexture2d);
	std::shared_ptr<Texture2DArray>	GetTextureArray(InGameTextureType type) const { return m_textureDiction[type].pTextureArray; }
	std::shared_ptr<TextureCube> GetIBLTextureCube(IBLTextureType type) const { return m_IBLCubeTextures[type]; }
	std::shared_ptr<Texture2D> GetIBLTexture2D(IBLTextureType type) const { return m_IBL2DTextures[type]; }
	void InitIBLTextures(const gli::texture_cube& skyBoxTex);
	bool GetTextureIndex(InGameTextureType type, const std::string& textureName, uint32_t& textureIndex);

protected:
	bool Init(const std::shared_ptr<GlobalTextures>& pSelf);
	void InitTextureDiction();
	void InitIBLTextures();
	void InitIrradianceTexture();
	void InitPrefilterEnvTexture();
	void InitBRDFLUTTexture();

protected:
	std::vector<TextureArrayDesc>				m_textureDiction;
	std::vector<std::shared_ptr<TextureCube>>	m_IBLCubeTextures;
	std::vector<std::shared_ptr<Texture2D>>		m_IBL2DTextures;
};