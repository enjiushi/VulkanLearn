#include "SwapChainImage.h"
#include "GlobalDeviceObjects.h"
#include "SwapChain.h"
#include "CommandPool.h"
#include "Queue.h"
#include "CommandBuffer.h"

bool SwapChainImage::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SwapChainImage>& pSelf, VkImage rawImageHandle)
{
	m_info = {};
	m_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	m_info.format = pDevice->GetPhysicalDevice()->GetSurfaceFormat().format;
	m_info.arrayLayers = 1;
	m_info.extent.depth = 1;
	m_info.extent.width = pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.width;
	m_info.extent.height = pDevice->GetPhysicalDevice()->GetSurfaceCap().currentExtent.height;
	m_info.imageType = VK_IMAGE_TYPE_2D;
	m_info.mipLevels = 1;
	m_info.samples = VK_SAMPLE_COUNT_1_BIT;

	m_shouldDestoryRawImage = false;

	if (!Image::Init(pDevice, pSelf, rawImageHandle))
		return false;

	return true;
}

std::vector<std::shared_ptr<SwapChainImage>> SwapChainImage::Create(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<SwapChain>& pSwapChain)
{
	std::vector<VkImage> rawImgList;
	uint32_t count;
	CHECK_VK_ERROR((pSwapChain->GetGetSwapchainImagesFuncPtr())(pDevice->GetDeviceHandle(), pSwapChain->GetDeviceHandle(), &count, nullptr));
	rawImgList.resize(count);
	CHECK_VK_ERROR((pSwapChain->GetGetSwapchainImagesFuncPtr())(pDevice->GetDeviceHandle(), pSwapChain->GetDeviceHandle(), &count, rawImgList.data()));

	std::vector<std::shared_ptr<SwapChainImage>> imgList;
	for (uint32_t i = 0; i < count; i++)
	{
		imgList.push_back(Create(pDevice, rawImgList[i]));
	}
	return imgList;
}

std::shared_ptr<SwapChainImage> SwapChainImage::Create(const std::shared_ptr<Device>& pDevice, VkImage rawImageHandle)
{
	std::shared_ptr<SwapChainImage> pImage = std::make_shared<SwapChainImage>();
	if (pImage.get() && pImage->Init(pDevice, pImage, rawImageHandle))
		return pImage;
	return nullptr;
}

void SwapChainImage::EnsureImageLayout()
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = GlobalObjects()->GetMainThreadGraphicCmdPool()->AllocatePrimaryCommandBuffer();

	pCmdBuffer->StartPrimaryRecording();

	//Change image layout
	std::vector<VkImageMemoryBarrier> imgBarriers(1);
	imgBarriers[0] = {};
	imgBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarriers[0].srcAccessMask = 0;
	imgBarriers[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imgBarriers[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imgBarriers[0].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	imgBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imgBarriers[0].image = GetDeviceHandle();
	imgBarriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imgBarriers[0].subresourceRange.baseMipLevel = 0;
	imgBarriers[0].subresourceRange.levelCount = 1;
	imgBarriers[0].subresourceRange.baseArrayLayer = 0;
	imgBarriers[0].subresourceRange.layerCount = 1;

	pCmdBuffer->AttachBarriers
	(
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		{},
		{},
		imgBarriers
	);

	pCmdBuffer->EndPrimaryRecording();

	GlobalGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
}