#pragma once
#include "PerObjectUniforms.h"
#include "../vulkan/SwapChain.h"
#include "../vulkan/GlobalDeviceObjects.h"
#include "../vulkan/Buffer.h"
#include "UniformData.h"
#include "Material.h"

bool PerObjectUniforms::Init(const std::shared_ptr<PerObjectUniforms>& pSelf)
{
	if (!UniformDataStorage::Init(pSelf, sizeof(m_perObjectVariables)))
		return false;

	m_freeChunks.push_back({ 0, MAXIMUM_OBJECTS });
	return true;
}

std::shared_ptr<PerObjectUniforms> PerObjectUniforms::Create()
{
	std::shared_ptr<PerObjectUniforms> pPerObjectUniforms = std::make_shared<PerObjectUniforms>();
	if (pPerObjectUniforms.get() && pPerObjectUniforms->Init(pPerObjectUniforms))
		return pPerObjectUniforms;
	return nullptr;
}

void PerObjectUniforms::SetModelMatrix(uint32_t index, const Matrix4f& modelMatrix)
{
	m_perObjectVariables[index].modelMatrix = modelMatrix;
	SetDirty(index);
}

void PerObjectUniforms::SyncBufferDataInternal()
{
	GetBuffer()->UpdateByteStream(m_perObjectVariables, FrameMgr()->FrameIndex() * GetFrameOffset(), sizeof(m_perObjectVariables));
}

void PerObjectUniforms::SetDirty(uint32_t index)
{
	m_perObjectVariables[index].MVPN = UniformData::GetInstance()->GetPerFrameUniforms()->GetVPNMatrix() * m_perObjectVariables[index].modelMatrix;
	UniformDataStorage::SetDirty();
}

uint32_t PerObjectUniforms::AllocatePerObjectChunk()
{
	ASSERTION(m_freeChunks.size() != 0);

	uint32_t index = m_freeChunks[0].first++;
	if (m_freeChunks[0].first > m_freeChunks[0].second)
		m_freeChunks.erase(m_freeChunks.begin());

	return index;
}

void PerObjectUniforms::FreePreObjectChunk(uint32_t index, uint32_t start, uint32_t end)
{
	uint32_t midChunkIndex = (start + end) / 2;

	std::pair<bool, bool> val;
	val = std::make_pair<bool, bool>(index <= m_freeChunks[midChunkIndex].first, index >= m_freeChunks[midChunkIndex].first);

	int32_t left, right;

	// If index is already freed
	if (val.first && val.second)
		return;
	else if (val.first)
	{
		left = midChunkIndex - 1;
		right = midChunkIndex;
	}
	else
	{
		left = midChunkIndex;
		right = midChunkIndex + 1;
	}

	if ((left < 0 || right == m_freeChunks.size()) || 
		(index > m_freeChunks[left].second)	||
		(index < m_freeChunks[right].first))
		InsertIntoFreeChunk(index, midChunkIndex);
	else
	{
		if (right == midChunkIndex)
			FreePreObjectChunk(index, start, midChunkIndex);
		else
			FreePreObjectChunk(index, midChunkIndex, end);
	}
}

void PerObjectUniforms::InsertIntoFreeChunk(uint32_t index, uint32_t chunkIndex)
{
	if (index < m_freeChunks[chunkIndex].first)
	{
		if (index == m_freeChunks[chunkIndex].first - 1)
			m_freeChunks[chunkIndex].first--;
		else
			m_freeChunks.insert(m_freeChunks.begin() + chunkIndex, { index, index });
	}
	else if (index > m_freeChunks[chunkIndex].second)
	{
		if (index == m_freeChunks[chunkIndex].second + 1)
			m_freeChunks[chunkIndex].second++;
		else
			m_freeChunks.insert(m_freeChunks.begin() + chunkIndex + 1, { index, index });
	}

}

void PerObjectUniforms::FreePreObjectChunk(uint32_t index)
{
	FreePreObjectChunk(index, 0, m_freeChunks.size() - 1);
}

UniformVarList PerObjectUniforms::PrepareUniformVarList()
{
	return
	{
		DynamicShaderStorageBuffer,
		"PerFrameUniforms",
		{
			{ Mat4Unit, "modelMatrix" },
			{ Mat4Unit, "MVP" },
		}
	};
}


