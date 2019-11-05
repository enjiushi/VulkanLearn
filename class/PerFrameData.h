#pragma once

#include "../Maths/Matrix.h"
#include "../common/Singleton.h"
#include "UniformDataStorage.h"

class PerFrameBuffer : public PerFrameDataStorage
{
public:
	static std::shared_ptr<PerFrameBuffer> Create(uint32_t size);

	virtual ~PerFrameBuffer();

public:
	void* DataPtr() const { return m_pData; }

protected:
	bool Init(const std::shared_ptr<PerFrameBuffer>& pSelf, uint32_t size);

protected:
	void UpdateUniformDataInternal() override {}
	void SetDirtyInternal() override {}
	const void* AcquireDataPtr() const { return m_pData; }
	uint32_t AcquireDataSize() const override { return GetFrameOffset(); }

private:
	void*	m_pData;
};

class PerFrameData : public Singleton<PerFrameData>
{
public:
	class PerFrameDataKey
	{
	public:
		PerFrameDataKey(uint32_t inputKey) : key(inputKey) {}
		~PerFrameDataKey();

	private:
		uint32_t	key;

		friend class PerFrameData;
	};

public:
	std::shared_ptr<PerFrameDataKey> AllocateBuffer(uint32_t size);
	const std::shared_ptr<PerFrameBuffer>& GetPerFrameBuffer(const std::shared_ptr<PerFrameDataKey>& pKey) const { return m_storageBuffers[pKey->key]; }

	void SyncDataBuffer();

private:
	void DeallocateBuffer(uint32_t key);

private:
	std::vector<std::shared_ptr<PerFrameBuffer>>		m_storageBuffers;
};