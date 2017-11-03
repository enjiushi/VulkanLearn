#pragma once

#include "../Maths/Matrix.h"
#include "ChunkBasedUniforms.h"

class PerMaterialUniforms : public ChunkBasedUniforms
{
public:
	static std::shared_ptr<PerMaterialUniforms> Create(uint32_t numBytes);
	~PerMaterialUniforms();

public:
	UniformVarList PrepareUniformVarList() override { return UniformVarList(); }

	template <typename T>
	void SetParameter(uint32_t offset, T val)
	{
		memcpy_s(m_pData, m_numBytes, &val, sizeof(val));
	}

protected:
	bool Init(const std::shared_ptr<PerMaterialUniforms>& pSelf, uint32_t numBytes);

	void SyncBufferDataInternal() override;

protected:
	uint8_t*	m_pData;
};