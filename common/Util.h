#pragma once
#include <assert.h>
#include <vulkan.h>
#include <vector>

#define SAFE_DELETE(x) if ((x) != nullptr) { delete (x); (x) = nullptr; }

#ifdef _DEBUG
#define SEE_IF_GL_ERROR GLenum error = glGetError(); assert(error == GL_NO_ERROR);
#else
#define SEE_IF_GL_ERROR
#endif //_DEBUG

#define CREATE_OBJ(type, pRet) type* pRet = new type; \
if (!pRet || !pRet->Init()) \
{ \
	SAFE_DELETE(pRet); \
	return nullptr; \
}

#define EQUAL(type, x, y) ((((x) - (std::numeric_limits<type>::epsilon())) <= (y)) && (((x) + (std::numeric_limits<type>::epsilon())) >= (y)))

uint32_t GetVertexBytes(uint32_t vertexFormat);
uint32_t GetIndexBytes(VkIndexType indexType);
VkVertexInputBindingDescription GenerateBindingDesc(uint32_t bindingIndex, uint32_t vertexFormat);
std::vector<VkVertexInputAttributeDescription> GenerateAttribDesc(uint32_t bindingIndex, uint32_t vertexFormat, uint32_t vertexFormatInMem);