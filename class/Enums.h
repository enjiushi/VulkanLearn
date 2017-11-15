#pragma once
#include <vulkan.h>

enum VertexAttribFlag
{
	VAFPosition,
	VAFNormal,
	VAFColor,
	VAFTexCoord,
	VAFTangent,
	VACount
};

uint32_t GetVertexBytes(uint32_t vertexAttribFlag);
uint32_t GetIndexBytes(VkIndexType indexType);