#pragma once

#include "DeviceObjectBase.h"
#include "VKGPUSyncRes.h"
#include "Buffer.h"
#include "../Maths/Vector.h"
#include <gli\gli.hpp>

class SwapChain;
class MemoryKey;
class CommandBuffer;
class StagingBuffer;
class ImageView;
class Sampler;

typedef struct _GliImageWrapper
{
	std::vector<gli::texture>	textures;
}GliImageWrapper;

class Image : public VKGPUSyncRes
{
public:
	~Image();

public:
	VkImage GetDeviceHandle() const { return m_image; }
	const VkImageCreateInfo& GetImageInfo() const { return m_info; }
	virtual uint32_t GetMemoryProperty() const { return m_memProperty; }
	virtual VkMemoryRequirements GetMemoryReqirments() const;
	virtual void EnsureImageLayout();

	VkPipelineStageFlags GetAccessStages() const { return m_accessStages; }
	VkAccessFlags GetAccessFlags() const { return m_accessFlags; }
	uint32_t GetBytesPerPixel() const { return m_bytesPerPixel; }

	void UpdateByteStream(const GliImageWrapper& gliTex);
	void UpdateByteStream(const GliImageWrapper& gliTex, uint32_t layer);

	void CopyFromBuffer(const std::shared_ptr<Buffer>& pBuffer, const std::shared_ptr<CommandBuffer>& pCommandBuffer);

	virtual std::shared_ptr<ImageView> CreateDefaultImageView(bool isStorage = false) const;
	virtual std::shared_ptr<ImageView> CreateImageView(uint32_t mipLevel, bool isStorage = false) const;
	virtual std::shared_ptr<ImageView> CreateDepthSampleImageView() const;
	virtual std::shared_ptr<Sampler> CreateLinearRepeatSampler() const;
	virtual std::shared_ptr<Sampler> CreateNearestRepeatSampler() const;
	virtual std::shared_ptr<Sampler> CreateLinearClampToBorderSampler(VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE) const;
	virtual std::shared_ptr<Sampler> CreateLinearClampToEdgeSampler() const;

	virtual void InsertTexture(const gli::texture2d& texture, uint32_t layer);

	static VkImageAspectFlags AcquireImageAspectFlags(VkFormat format);
	void PrepareBarriers
	(
		VkAccessFlags				srcAccessFlags,
		VkImageLayout				srcImageLayout,
		VkAccessFlags				dstAccessFlags,
		VkImageLayout				dstImageLayout,
		std::vector<VkMemoryBarrier>&		memBarriers,
		std::vector<VkBufferMemoryBarrier>&	bufferMemBarriers,
		std::vector<VkImageMemoryBarrier>&	imageMemBarriers
	) override;

	void PrepareQueueReleaseBarrier
	(
		VkAccessFlags				srcAccessFlags,
		VkImageLayout				srcImageLayout,
		PhysicalDevice::QueueFamily	srcQueueFamily,
		PhysicalDevice::QueueFamily	dstQueueFamily,
		std::vector<VkMemoryBarrier>&		memBarriers,
		std::vector<VkBufferMemoryBarrier>&	bufferMemBarriers,
		std::vector<VkImageMemoryBarrier>&	imageMemBarriers
	) override;

	void PrepareQueueAcquireBarrier
	(
		VkAccessFlags				dstAccessFlags,
		VkImageLayout				dstImageLayout,
		PhysicalDevice::QueueFamily	srcQueueFamily,
		PhysicalDevice::QueueFamily	dstQueueFamily,
		std::vector<VkMemoryBarrier>&		memBarriers,
		std::vector<VkBufferMemoryBarrier>&	bufferMemBarriers,
		std::vector<VkImageMemoryBarrier>&	imageMemBarriers
	) override;

protected:
	virtual void BindMemory(VkDeviceMemory memory, uint32_t offset) const;

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, VkImage img);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, const VkImageCreateInfo& info, uint32_t memoryPropertyFlag);
	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<Image>& pSelf, const GliImageWrapper& gliTex, const VkImageCreateInfo& info, uint32_t memoryPropertyFlag);

	virtual std::shared_ptr<StagingBuffer> PrepareStagingBuffer(const GliImageWrapper& gliTex, const std::shared_ptr<CommandBuffer>& pCmdBuffer);
	virtual void ExecuteCopy(const GliImageWrapper& gliTex, const std::shared_ptr<StagingBuffer>& pStagingBuffer, const std::shared_ptr<CommandBuffer>& pCmdBuffer);
	virtual uint32_t ExecuteCopy(const GliImageWrapper& gliTex, uint32_t layer, const std::shared_ptr<StagingBuffer>& pStagingBuffer, uint32_t offset, const std::shared_ptr<CommandBuffer>& pCmdBuffer);

public:
	static std::shared_ptr<Image> CreateEmptyTexture
	(
		const std::shared_ptr<Device>& pDevice,
		const Vector3ui& size,
		uint32_t mipLevels,
		uint32_t layers,
		VkFormat format,
		VkImageLayout defaultLayout,
		VkImageUsageFlags usage,
		VkPipelineStageFlags stageFlag,
		VkAccessFlags accessFlag,
		VkImageViewCreateFlags createFlag = 0
	);

	static std::shared_ptr<Image> CreateTextureWithGLIImage
	(
		const std::shared_ptr<Device>& pDevice,
		const GliImageWrapper& GLIImage,
		VkFormat format,
		VkImageLayout defaultLayout,
		VkImageUsageFlags usage,
		VkPipelineStageFlags stageFlag,
		VkAccessFlags accessFlag,
		VkImageViewCreateFlags createFlag = 0
	);

	// Factory methods:
	// Texture2D:
	static std::shared_ptr<Image> CreateTexture2D(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format);
	static std::shared_ptr<Image> CreateTexture2D(const std::shared_ptr<Device>& pDevice, const GliImageWrapper& gliTex2d, VkFormat format);
	static std::shared_ptr<Image> CreateEmptyTexture2D(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, VkFormat format);
	static std::shared_ptr<Image> CreateEmptyTexture2DForCompute(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, VkFormat format);
	static std::shared_ptr<Image> CreateOffscreenTexture2D(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, VkFormat format);
	static std::shared_ptr<Image> CreateOffscreenTexture2D(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, VkFormat format, VkImageLayout layout, VkImageUsageFlagBits extraUsage);
	static std::shared_ptr<Image> CreateOffscreenTexture2D(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, VkFormat format, VkImageLayout layout);
	static std::shared_ptr<Image> CreateMipmapOffscreenTexture2D(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, VkFormat format, VkImageLayout layout);

	// Texture2D Array:
	static std::shared_ptr<Image> CreateTexture2DArray(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format);
	static std::shared_ptr<Image> CreateTexture2DArray(const std::shared_ptr<Device>& pDevice, const GliImageWrapper& gliTextureArray, VkFormat format);
	static std::shared_ptr<Image> CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, uint32_t layers, VkFormat format);
	static std::shared_ptr<Image> CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, uint32_t layers, VkFormat format, VkImageLayout defaultLayout);
	static std::shared_ptr<Image> CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, uint32_t mipLevels, uint32_t layers, VkFormat format);
	static std::shared_ptr<Image> CreateEmptyTexture2DArray(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, uint32_t mipLevels, uint32_t layers, VkFormat format, VkImageLayout defaultLayout);
	static std::shared_ptr<Image> CreateMipmapOffscreenTexture2DArray(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, uint32_t layers, VkFormat format);

	static std::shared_ptr<Image> CreateEmptyTexture3D(const std::shared_ptr<Device>& pDevice, const Vector3ui& size, VkFormat format, VkImageLayout defaultLayout);

	// TextureCube:
	static std::shared_ptr<Image> CreateCubeTexture(const std::shared_ptr<Device>& pDevice, std::string path, VkFormat format);
	static std::shared_ptr<Image> CreateEmptyCubeTexture(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, VkFormat format);
	static std::shared_ptr<Image> CreateEmptyCubeTexture(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, uint32_t mipLevels, VkFormat format);
	static std::shared_ptr<Image> CreateEmptyCubeTexture(const std::shared_ptr<Device>& pDevice, const Vector2ui& size, uint32_t mipLevels, VkFormat format, VkImageLayout layout, VkImageUsageFlagBits extraUsage);

	// DepthStencilBuffer:
	static std::shared_ptr<Image> CreateDepthStencilBuffer(const std::shared_ptr<Device>& pDevice, VkFormat format, const Vector2ui& size);
	static std::shared_ptr<Image> CreateDepthStencilBuffer(const std::shared_ptr<Device>& pDevice, VkFormat format, const Vector2ui& size, VkImageUsageFlags usage);
	static std::shared_ptr<Image> CreateDepthStencilBuffer(const std::shared_ptr<Device>& pDevice, VkFormat format);
	static std::shared_ptr<Image> CreateDepthStencilInputAttachment(const std::shared_ptr<Device>& pDevice, VkFormat format);
	static std::shared_ptr<Image> CreateDepthStencilSampledAttachment(const std::shared_ptr<Device>& pDevice, VkFormat format, const Vector2ui& size);

protected:
	VkImage						m_image;
	VkImageCreateInfo			m_info;

	bool						m_shouldDestoryRawImage = true;

	uint32_t					m_memProperty;

	std::shared_ptr<MemoryKey>	m_pMemKey;

	VkPipelineStageFlags		m_accessStages;
	VkAccessFlags				m_accessFlags;
	uint32_t					m_bytesPerPixel;

	friend class DeviceMemoryManager;
};