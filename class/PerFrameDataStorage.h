#pragma once

#include "../Maths/Matrix.h"
#include "../Base/Base.h"

class BufferBase;
class UniformBuffer;
class ShaderStorageBuffer;

enum MaterialVariableType;

class PerFrameDataStorage : public SelfRefBase<PerFrameDataStorage>
{
public:
	enum StorageType
	{
		// Normal uniform buffer, host visible, coherent
		Uniform,

		// Normal shader storage buffer, host visible, coherent, larger
		ShaderStorage,

		// Host visible, coherent, used for streaming any kind
		Streaming,
		StorageTypeCount
	};

protected:
	bool Init(const std::shared_ptr<PerFrameDataStorage>& pSelf, uint32_t numBytes, StorageType storageType);

public:
	uint32_t GetFrameOffset() const { return m_frameOffset; }
	void SyncBufferData();
	std::shared_ptr<BufferBase> GetBuffer() const;

protected:
	virtual void UpdateUniformDataInternal() = 0;
	virtual void SyncBufferDataInternal();
	virtual void SetDirtyInternal() = 0;
	virtual const void* AcquireDataPtr() const = 0;
	virtual uint32_t AcquireDataSize() const = 0;
	void SetDirty();

protected:
	std::shared_ptr<BufferBase>	m_pBuffer;
	StorageType					m_storageType;
	std::vector<bool>			m_pendingSync;
	uint32_t					m_pendingSyncCount;
	uint32_t					m_frameOffset;
};