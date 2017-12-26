#pragma once

#include "DeviceObjectBase.h"
#include <gli\gli.hpp>

class SwapChain;
class MemoryKey;
class CommandBuffer;
class StagingBuffer;

typedef struct _GliImageWrapper
{
	std::vector<gli::texture>	textures;
}GliImageWrapper;

class Image : public DeviceObjectBase<Image>
{
public:
	~Image();

public:
	VkImage GetDeviceHandle() const { return m_image; }
	VkDescriptorImageInfo GetDescriptorInfo() const { return m_descriptorImgInfo; }
	const VkImageCreateInfo& GetImageInfo() const { return m_info; }
	virtual uint32_t GetMemoryProperty() const { return m_memProperty; }
	virtual VkMemoryRequirements GetMemoryReqirments() const;
	virtual VkImageViewCreateInfo GetViewInfo() const { return m_viewInfo; }
	virtual VkImageView GetViewDeviceHandle() const { return m_view; }
	virtual void EnsureImageLayout();

	VkPipelineStageFlags GetAccessStages() const { return m_accessStages; }
	VkAccessFlags GetAccessFlags() const { return m_accessFlags; }
	uint32_t GetBytesPerPixel() const { return m_bytesPerPixel; }

	void UpdateByteStream(const GliImageWrapper& gliTex);
	void UpdateByteStream(const GliImageWrapper& gliTex, uint32_t layer);

protected:
	virtual void BindMemory(VkDeviceMemory memory, uint32_t offset) const;

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, VkImage img);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, const VkImageCreateInfo& info, uint32_t memoryPropertyFlag);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, const GliImageWrapper& gliTex, const VkImageCreateInfo& info, uint32_t memoryPropertyFlag);

	virtual void CreateImageView();
	virtual void CreateSampler();

	virtual std::shared_ptr<StagingBuffer> PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer) = 0;
	virtual void ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) = 0;
	virtual void ExecuteCopy(const GliImageWrapper& gliTex, uint32_t layer, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) = 0;

protected:
	VkImage						m_image;
	VkImageCreateInfo			m_info;

	bool						m_shouldDestoryRawImage = true;

	// Put image view here, maybe for now, since I don't need one image with multiple view at this moment
	VkImageViewCreateInfo		m_viewInfo;
	VkImageView					m_view;
	VkSamplerCreateInfo			m_samplerInfo;
	VkSampler					m_sampler;

	VkDescriptorImageInfo		m_descriptorImgInfo;

	uint32_t					m_memProperty;

	std::shared_ptr<MemoryKey>	m_pMemKey;

	VkPipelineStageFlags		m_accessStages;
	VkAccessFlags				m_accessFlags;
	uint32_t					m_bytesPerPixel;

	friend class DeviceMemoryManager;
};