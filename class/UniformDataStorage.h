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
protected:
	static const uint32_t MAXIMUM_OBJECTS = 1024;

public:
	bool Init(const std::shared_ptr<UniformDataStorage>& pSelf, uint32_t numBytes, bool perObject);

public:
	uint32_t GetFrameOffset() const { return m_frameOffset; }
	void SyncBufferData();
	std::shared_ptr<Buffer> GetBuffer() const;

protected:
	virtual void SyncBufferDataInternal() = 0;
	virtual void SetDirty();

protected:
	std::shared_ptr<UniformBuffer>			m_pUniformBuffer;
	std::shared_ptr<ShaderStorageBuffer>	m_pShaderStorageBuffer;
	uint32_t								m_pendingSyncCount;
	uint32_t								m_frameOffset;
};