#pragma once
#include "Material.h"
#include "FrameBufferDiction.h"
#include "RenderPassDiction.h"

class Image;

class CustomizedComputeMaterial : public Material
{
public:
	typedef struct _Variables
	{
		std::wstring	shaderPath;
		Vector3ui		groupSize;

		std::vector<std::shared_ptr<Image>>	textures;
		std::vector<Vector4ui>				textureSubresRanges;
	}Variables;

public:
	static std::shared_ptr<CustomizedComputeMaterial> CreateMaterial(const CustomizedComputeMaterial::Variables& variables);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0, bool overrideVP = false) override {}

protected:
	bool Init(const std::shared_ptr<CustomizedComputeMaterial>& pSelf, const CustomizedComputeMaterial::Variables& variables);

	void CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout) override;
	void CustomizePoolSize(std::vector<uint32_t>& counts) override;

private:
	Variables	m_variables;
};