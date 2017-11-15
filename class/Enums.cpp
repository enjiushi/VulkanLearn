#include "Enums.h"
#include "../common/Macros.h"

uint32_t GetVertexBytes(uint32_t vertexAttribFlag)
{
	uint32_t vertexByte = 0;
	if (vertexAttribFlag & (1 << VAFPosition))
	{
		vertexByte += 3 * sizeof(float);
	}
	if (vertexAttribFlag & (1 << VAFNormal))
	{
		vertexByte += 3 * sizeof(float);
	}
	if (vertexAttribFlag & (1 << VAFColor))
	{
		vertexByte += 4 * sizeof(float);
	}
	if (vertexAttribFlag & (1 << VAFTexCoord))
	{
		vertexByte += 2 * sizeof(float);
	}
	if (vertexAttribFlag & (1 << VAFTangent))
	{
		vertexByte += 3 * sizeof(float);
	}
	return vertexByte;
}

uint32_t GetIndexBytes(VkIndexType indexType)
{
	switch (indexType)
	{
	case VK_INDEX_TYPE_UINT16: return 2;
	case VK_INDEX_TYPE_UINT32: return 4;
	default: ASSERTION(false);
	}
	return 0;
}