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
	void SetParameter(uint32_t parameterChunkIndex, uint32_t parameterOffset, T val)
	{
		// m_pData : GetFrameOffset()            GetFrameOffset()            GetFrameOffset()
		//           =======================     =======================     =======================
		//                                            |
		//                              chunkIndex * m_perMaterialInstanceBytes
		//                                               |
		//                                              offset
		memcpy_s(m_pData + GetFrameOffset() + parameterChunkIndex * m_perMaterialInstanceBytes + parameterOffset, GetFrameOffset() - parameterChunkIndex * m_perMaterialInstanceBytes - parameterOffset, &val, sizeof(val));
		SetDirty();
	}

	template <typename T>
	T GetParameter(uint32_t parameterChunkIndex, uint32_t parameterOffset)
	{
		//return m_pMaterial->GetParameter(bindingIndex, parameterIndex);
		T ret;
		memcpy_s(&ret, sizeof(ret), m_pData + GetFrameOffset() + parameterChunkIndex * m_perMaterialInstanceBytes + parameterOffset, sizeof(T));
		return ret;
	}

protected:
	bool Init(const std::shared_ptr<PerMaterialUniforms>& pSelf, uint32_t numBytes);

	void SyncBufferDataInternal() override;

protected:
	uint8_t*	m_pData;
	uint32_t	m_perMaterialInstanceBytes;
};