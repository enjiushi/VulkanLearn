#pragma once

#include "Image.h"
#include <string>
#include <gli\gli.hpp>

class Texture2D : public Image
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2D>& pSelf, const gli::texture2d& gliTex2d, VkFormat format);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2D>& pSelf, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
	void CreateImageView(VkImageView* pView, VkImageViewCreateInfo& viewCreateInfo) override;
	void UpdateByteStream(const gli::texture2d& gliTex2d);
	void CreateSampler();

public:
	void EnsureImageLayout() override;
	VkDescriptorImageInfo GetDescriptorInfo() const { return m_descriptorImgInfo; }

public:
	static std::shared_ptr<Texture2D> Create(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format);
	static std::shared_ptr<Texture2D> CreateEmptyTexture(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, VkFormat format);
	static std::shared_ptr<Texture2D> CreateOffscreenTexture(const std::shared_ptr<Device>& pDevice, uint32_t width, uint32_t height, VkFormat format);

private:
	VkSampler				m_sampler;
	VkImageLayout			m_layout;
	VkDescriptorImageInfo	m_descriptorImgInfo;
};