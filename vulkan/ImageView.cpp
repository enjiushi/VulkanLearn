#include "ImageView.h"
#include "GlobalDeviceObjects.h"

ImageView::~ImageView()
{
	if (m_imageView)
		vkDestroyImageView(GetDevice()->GetDeviceHandle(), m_imageView, nullptr);
}

bool ImageView::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<ImageView>& pSelf, const VkImageViewCreateInfo& info)
{
	if (!DeviceObjectBase::Init(pDevice, pSelf))
		return false;

	RETURN_FALSE_VK_RESULT(vkCreateImageView(m_pDevice->GetDeviceHandle(), &info, nullptr, &m_imageView));

	return true;
}

std::shared_ptr<ImageView> ImageView::Create(const std::shared_ptr<Device>& pDevice, const VkImageViewCreateInfo& info)
{
	std::shared_ptr<ImageView> pImageView = std::make_shared<ImageView>();
	if (pImageView.get() && pImageView->Init(pDevice, pImageView, info))
		return pImageView;
	return nullptr;
}