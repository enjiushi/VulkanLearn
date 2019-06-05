#pragma once

#include "../Maths/Matrix.h"
#include "../Base/Base.h"
#include "UniformBase.h"

class DescriptorSet;
class Buffer;
class UniformBuffer;
class ShaderStorageBuffer;

enum MaterialVariableType;

class UniformDataStorage : public UniformBase
{
public:
	bool Init(const std::shared_ptr<UniformDataStorage>& pSelf, uint32_t numBytes, bool perObject);

public:
	uint32_t GetFrameOffset() const { return m_frameOffset; }
	void SyncBufferData();
	std::shared_ptr<Buffer> GetBuffer() const;

protected:
	virtual void UpdateUniformDataInternal() = 0;
	virtual void SyncBufferDataInternal();
	virtual void SetDirtyInternal() = 0;
	virtual const void* AcquireDataPtr() const = 0;
	virtual uint32_t AcquireDataSize() const = 0;
	void SetDirty();

protected:
	std::shared_ptr<UniformBuffer>			m_pUniformBuffer;
	std::shared_ptr<ShaderStorageBuffer>	m_pShaderStorageBuffer;
	uint32_t								m_pendingSyncCount;
	uint32_t								m_frameOffset;
};