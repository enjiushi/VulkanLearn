#pragma once

#include "Image.h"

class DepthStencilBuffer : public Image
{
public:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<DepthStencilBuffer>& pSelf, const VkImageCreateInfo& info);

public:
	static std::shared_ptr<DepthStencilBuffer> Create(const std::shared_ptr<Device>& pDevice, VkFormat format, uint32_t width, uint32_t height);
	static std::shared_ptr<DepthStencilBuffer> Create(const std::shared_ptr<Device>& pDevice, VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usage);
	static std::shared_ptr<DepthStencilBuffer> Create(const std::shared_ptr<Device>& pDevice, VkFormat format);
	static std::shared_ptr<DepthStencilBuffer> CreateInputAttachment(const std::shared_ptr<Device>& pDevice, VkFormat format);
	static std::shared_ptr<DepthStencilBuffer> CreateSampledAttachment(const std::shared_ptr<Device>& pDevice, VkFormat format, uint32_t width, uint32_t height);

public:
	std::shared_ptr<ImageView> CreateDefaultImageView() const override;
	std::shared_ptr<ImageView> CreateDepthSampleImageView() const;

protected:
	std::shared_ptr<StagingBuffer> PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override { return nullptr; };
	void ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override {};
	void ExecuteCopy(const GliImageWrapper& gliTex, uint32_t layer, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer) override {}
};