#pragma once

#include "Image.h"
#include <string>
#include <gli\gli.hpp>

class Texture2D : public Image
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2D>& pSelf, const gli::texture2d& gliTex2d, VkFormat format);
	void CreateImageView(VkImageView* pView, VkImageViewCreateInfo& viewCreateInfo) override;
	virtual void UpdateByteStream(const gli::texture2d& gliTex2d);

public:
	void EnsureImageLayout() override;

public:
	static std::shared_ptr<Texture2D> Create(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format);
};