#include "Texture2D.h"
#include "GlobalDeviceObjects.h"
#include "CommandPool.h"
#include "Queue.h"
#include "CommandBuffer.h"
#include "StagingBuffer.h"

bool Texture2D::Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Texture2D>& pSelf, const gli::texture2d& gliTex2d, VkFormat format)
{
	uint32_t width = static_cast<uint32_t>(gliTex2d[0].extent().x);
	uint32_t height = static_cast<uint32_t>(gliTex2d[0].extent().y);
	uint32_t mipLevels = static_cast<uint32_t>(gliTex2d.levels());

	VkImageCreateInfo textureCreateInfo = {};
	textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureCreateInfo.format = format;
	textureCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	textureCreateInfo.arrayLayers = 1;
	textureCreateInfo.extent.depth = 1;
	textureCreateInfo.extent.width = width;
	textureCreateInfo.extent.height = height;
	textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	textureCreateInfo.mipLevels = mipLevels;
	textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (!Image::Init(pDevice, pSelf, textureCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		return false;

	UpdateByteStream(gliTex2d);

	return true;
}

std::shared_ptr<Texture2D> Texture2D::Create(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format)
{
	gli::texture2d gliTex2d(gli::load(path.c_str()));
	std::shared_ptr<Texture2D> pTexture = std::make_shared<Texture2D>();
	if (pTexture.get() && pTexture->Init(pDevice, pTexture, gliTex2d, format))
		return pTexture;
	return nullptr;
}

void Texture2D::UpdateByteStream(const gli::texture2d& gliTex2d)
{
	std::shared_ptr<StagingBuffer> pStagingBuffer = StagingBuffer::Create(m_pDevice, gliTex2d.size());
	pStagingBuffer->UpdateByteStream(gliTex2d.data(), 0, gliTex2d.size(), (VkPipelineStageFlagBits)0, 0);

	std::shared_ptr<CommandBuffer> pCmdBuffer = MainThreadPool()->AllocatePrimaryCommandBuffer();
	VkCommandBuffer cmdBuffer = pCmdBuffer->GetDeviceHandle();
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	// Barrier for host data copy & transfer src
	VkBufferMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	barrier.buffer = pStagingBuffer->GetDeviceHandle();
	barrier.offset = 0;
	barrier.size = gliTex2d.size();

	vkCmdPipelineBarrier(cmdBuffer,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		1, &barrier,
		0, nullptr);

	//Barrier for layout change from undefined to transfer dst
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = gliTex2d.levels();
	subresourceRange.layerCount = 1;

	VkImageMemoryBarrier imgBarrier = {};
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.image = GetDeviceHandle();
	imgBarrier.subresourceRange = subresourceRange;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imgBarrier.srcAccessMask = 0;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	vkCmdPipelineBarrier(pCmdBuffer->GetDeviceHandle(),
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imgBarrier);

	// Prepare copy info
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (uint32_t i = 0; i < gliTex2d.levels(); i++)
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = i;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = gliTex2d[i].extent().x;
		bufferCopyRegion.imageExtent.height = gliTex2d[i].extent().y;
		bufferCopyRegion.imageExtent.depth = gliTex2d[i].extent().z;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);

		offset += static_cast<uint32_t>(gliTex2d[i].size());
	}

	// Do copy
	vkCmdCopyBufferToImage(pCmdBuffer->GetDeviceHandle(),
		pStagingBuffer->GetDeviceHandle(),
		GetDeviceHandle(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		bufferCopyRegions.size(),
		bufferCopyRegions.data());

	// Change layout back to shader read
	imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(pCmdBuffer->GetDeviceHandle(),
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imgBarrier);

	CHECK_VK_ERROR(vkEndCommandBuffer(pCmdBuffer->GetDeviceHandle()));

	GlobalGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
}

void Texture2D::EnsureImageLayout()
{
	std::shared_ptr<CommandBuffer> pCmdBuffer = GlobalObjects()->GetMainThreadCmdPool()->AllocatePrimaryCommandBuffer();
	VkCommandBuffer cmdBuffer = pCmdBuffer->GetDeviceHandle();
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	//Change image layout
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = GetDeviceHandle();
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barrier.dstQueueFamilyIndex = m_pDevice->GetPhysicalDevice()->GetGraphicQueueIndex();
	barrier.srcAccessMask = 0;
	barrier.srcQueueFamilyIndex = m_pDevice->GetPhysicalDevice()->GetGraphicQueueIndex();
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;

	vkCmdPipelineBarrier(cmdBuffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	CHECK_VK_ERROR(vkEndCommandBuffer(cmdBuffer));

	GlobalObjects()->GetGraphicQueue()->SubmitCommandBuffer(pCmdBuffer, nullptr, true);
}

void Texture2D::CreateImageView(VkImageView* pView, VkImageViewCreateInfo& viewCreateInfo)
{
	//Create depth stencil image view
	viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.image = m_image;
	viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewCreateInfo.format = m_info.format;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = m_info.mipLevels;

	CHECK_VK_ERROR(vkCreateImageView(m_pDevice->GetDeviceHandle(), &viewCreateInfo, nullptr, pView));
}