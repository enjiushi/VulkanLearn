#pragma once

#include "Buffer.h"
#include "GlobalDeviceObjects.h"

class SharedBufferManager;
class SharedBuffer;

class BufferKey
{
public:
	static std::shared_ptr<BufferKey> Create(const std::shared_ptr<SharedBufferManager>& pSharedBufMgr, uint32_t key);
	~BufferKey();

	std::shared_ptr<SharedBufferManager> GetSharedBufferMgr() const { return m_pSharedBufMgr; }

private:
	bool Init(const std::shared_ptr<SharedBufferManager>& pSharedBufMgr, uint32_t key);

private:
	uint32_t								m_key;
	std::shared_ptr<SharedBufferManager>	m_pSharedBufMgr;

	friend class SharedBufferManager;
};

class SharedBufferManager : public DeviceObjectBase<SharedBufferManager>
{
protected:
	bool Init(const std::shared_ptr<Device>& pDevice, 
		const std::shared_ptr<SharedBufferManager>& pSelf,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlagBits memFlag,
		uint32_t numBytes);

	void FreeBuffer(uint32_t index);

public:
	static std::shared_ptr<SharedBufferManager> Create(const std::shared_ptr<Device>& pDevice,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlagBits memFlag,
		uint32_t numBytes);

public:
	std::shared_ptr<Buffer> GetBuffer() const { return m_pBuffer; }
	VkBuffer GetDeviceHandle() const { return m_pBuffer->GetDeviceHandle(); }
	std::shared_ptr<BufferKey> AllocateBuffer(uint32_t numBytes);
	uint32_t GetOffset(const std::shared_ptr<BufferKey>& pBufKey);
	VkDescriptorBufferInfo GetBufferDesc(const std::shared_ptr<BufferKey>& pBufKey);

	// Since m_pBuffer is used as internal buffer for shared buffers, we cannot directly use it, as many buffer specific member variables are missing
	// So I add one more input parameter "pWrapperBuffer", which wrappers "m_pBuffer" and behave exactly like a shared buffer with its member variable inited properly
	// Therefore other classes can access this shared buffer correctly
	void UpdateByteStream(const void* pData, const std::shared_ptr<Buffer>& pWrapperBuffer, const std::shared_ptr<BufferKey>& pBufKey, uint32_t offset, uint32_t numBytes);
	void UpdateByteStream(const void* pData, const std::shared_ptr<SharedBuffer>& pWrapperBuffer, const std::shared_ptr<BufferKey>& pBufKey, uint32_t offset, uint32_t numBytes);

protected:
	std::shared_ptr<Buffer>					m_pBuffer;

	// Where buffer chunks located, one after another
	std::vector<VkDescriptorBufferInfo>		m_bufferTable;

	// Key is BufferKey's m_key, and value is m_bufferTable's index
	std::map<uint32_t, uint32_t>			m_lookupTable;

	// After allocating a buffer key, this static variable is increased by one
	static uint32_t m_numAllocatedKeys;

	friend class BufferKey;
};