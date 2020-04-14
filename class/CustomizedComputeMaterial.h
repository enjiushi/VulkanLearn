#pragma once
#include "Material.h"
#include "FrameBufferDiction.h"
#include "RenderPassDiction.h"
#include "../vulkan/DescriptorSet.h"

class Image;

class CustomizedComputeMaterial : public Material
{
public:
	typedef struct _TextureBarrier
	{
		bool								enableBarrier;
		VkPipelineStageFlags				srcPipelineStages;
		VkImageLayout						oldImageLayout;
		VkAccessFlags						srcAccessFlags;

		VkPipelineStageFlags				dstPipelineStages;
		VkImageLayout						newImageLayout;
		VkAccessFlags						dstAccessFlags;
	}TextureBarrier;

	typedef struct _TextureUnit
	{
		uint32_t							bindingIndex;

		std::vector<CombinedImage>			textures;
		VkImageAspectFlags					aspectMask;
		Vector4ui							textureSubresRange;	// Base miplevel, mipLevel count, base array layer, array layer count
		bool								isStorageImage;		// Storage image is for compute

		// Barrier info
		enum TextureSelector
		{
			BY_FRAME,			// A texture will be selected by frame index
			BY_PINGPONG,		// A texture will be selected by pingpoing index
			BY_NEXTPINGPONG,	// A texture will be selected by next pingpoing index
			ALL,				// All textures will be selected
			NONE,				// No textures will be selected, no need for barriers
			COUNT
		}textureSelector;

		TextureBarrier						textureBarrier[Material::BarrierInsertionPoint::COUNT];

	}TextureUnit;

	typedef struct _Variables
	{
		std::wstring	shaderPath;
		Vector3ui		groupSize;

		std::vector<TextureUnit>	textureUnits;

		// Push constants
		std::vector<uint8_t>	pushConstantData;

		std::function<void(const std::shared_ptr<CommandBuffer>&, uint32_t)>	customFunctionAfterDispatch = 
			[](const std::shared_ptr<CommandBuffer>& pCmdBuf, uint32_t pingpong) {};
	}Variables;

public:
	static std::shared_ptr<CustomizedComputeMaterial> CreateMaterial(const CustomizedComputeMaterial::Variables& variables);

public:
	void Draw(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0, bool overrideVP = false) override {}

protected:
	bool Init(const std::shared_ptr<CustomizedComputeMaterial>& pSelf, const CustomizedComputeMaterial::Variables& variables);

	void CustomizeMaterialLayout(std::vector<UniformVarList>& materialLayout) override;
	void CustomizePoolSize(std::vector<uint32_t>& counts) override;
	void CustomizeCommandBuffer(const std::shared_ptr<CommandBuffer>& pCmdBuf, const std::shared_ptr<FrameBuffer>& pFrameBuffer, uint32_t pingpong = 0) override;

	void AfterRenderPass(const std::shared_ptr<CommandBuffer>& pCmdBuf, uint32_t pingpong) override;
	void AttachResourceBarriers(const std::shared_ptr<CommandBuffer>& pCmdBuffer, BarrierInsertionPoint barrierInsertionPoint, uint32_t pingpong = 0) override;

	void UpdatePushConstantDataInternal(const void* pData, uint32_t size) override;

	static void AssembleBarrier(const TextureUnit& textureUnit, uint32_t textureIndex, BarrierInsertionPoint barrierInsertPoint, VkImageMemoryBarrier& barrier, VkImageSubresourceRange& subresRange);

private:
	Variables	m_variables;
};