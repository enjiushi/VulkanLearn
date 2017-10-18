#pragma once

#include "DeviceObjectBase.h"
#include <map>

class Buffer;
class Image;

class MemoryKey
{
public:
	static std::shared_ptr<MemoryKey> Create(bool bufferOrImage);
	~MemoryKey();
private:
	bool Init(bool bufferOrImage);

private:
	uint32_t		m_key;
	bool			m_bufferOrImage;		//true: buffer, false: image
	static uint32_t m_allocatedKeys;

	friend class DeviceMemoryManager;
};

class DeviceMemoryManager : public DeviceObjectBase<DeviceMemoryManager>
{
	typedef struct _MemoryNode
	{
		uint32_t				numBytes = 0;
		VkDeviceMemory			memory;
		void*					pData = nullptr;
		std::vector<uint32_t>	bindingList;
	}MemoryNode;

	typedef struct _BindingInfo
	{
		uint32_t	typeIndex;
		uint32_t	startByte = 0;
		uint32_t	numBytes = 0;
		void*		pData = nullptr;
	}BindingInfo;

public:
	static const uint32_t MEMORY_ALLOCATE_INC = 1024 * 1024 * 512;

public:
	~DeviceMemoryManager();

	bool Init(const std::shared_ptr<Device>& pDevice, const std::shared_ptr<DeviceMemoryManager>& pSelf);

public:
	static std::shared_ptr<DeviceMemoryManager> Create(const std::shared_ptr<Device>& pDevice);
	std::shared_ptr<MemoryKey> AllocateBufferMemChunk(const std::shared_ptr<Buffer>& pBuffer, uint32_t memoryPropertyBits, const void* pData = nullptr);
	std::shared_ptr<MemoryKey> AllocateImageMemChunk(const std::shared_ptr<Image>& pImage, uint32_t memoryPropertyBits, const void* pData = nullptr);
	bool UpdateBufferMemChunk(const std::shared_ptr<MemoryKey>& pMemKey, uint32_t memoryPropertyBits, const void* pData, uint32_t offset, uint32_t numBytes);
	bool UpdateImageMemChunk(const std::shared_ptr<MemoryKey>& pMemKey, uint32_t memoryPropertyBits, const void* pData, uint32_t offset, uint32_t numBytes);
	void* GetDataPtr(const std::shared_ptr<MemoryKey>& pMemKey, uint32_t offset, uint32_t numBytes);

protected:
	void AllocateBufferMemory(uint32_t key, uint32_t numBytes, uint32_t memoryTypeBits, uint32_t memoryPropertyBits, uint32_t& typeIndex, uint32_t& offset);
	bool FindFreeBufferMemoryChunk(uint32_t key, uint32_t typeIndex, uint32_t numBytes, uint32_t& offset);
	void FreeBufferMemChunk(uint32_t key);

	void AllocateImageMemory(uint32_t key, uint32_t numBytes, uint32_t memoryTypeBits, uint32_t memoryPropertyBits, uint32_t& typeIndex, uint32_t& offset);
	void FreeImageMemChunk(uint32_t key);

	void UpdateMemoryChunk(VkDeviceMemory memory, uint32_t offset, uint32_t numBytes, void* pDst, const void* pData);
	void ReleaseMemory();

protected:
	std::map<uint32_t, MemoryNode>		m_bufferMemPool;
	std::map<uint32_t, MemoryNode>		m_imageMemPool;
	std::map<uint32_t, BindingInfo>		m_bufferBindingTable;

	friend class MemoryKey;
};