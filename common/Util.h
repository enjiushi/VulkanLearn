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

// 1023 is the bias of exponent bits
static const uint64_t zeroExponent = 1023;
// 52 is the count of fraction bits of double
static const uint64_t fractionBits = 52;
// 11 is the count of exponent bits of double
static const uint64_t exponentBits = 11;
// Extra one is the invisible one of double that is not in fraction bits for normal double
static const uint64_t extraOne = (1ull << fractionBits);
// Fraction mask is used to extract only fraction bits of a double
static const uint64_t fractionMask = (1ull << fractionBits) - 1;
// Exponent mask is used to extract only exponent bits of a double(You have to do right shift of "fractionBits" after)
static const uint64_t exponentMask = ((1ull << exponentBits) - 1) << fractionBits;

uint32_t GetVertexBytes(uint32_t vertexFormat);
uint32_t GetIndexBytes(VkIndexType indexType);

// There's mechanism that handles mesh data store and binding by default
// One could add more customized vertex buffers, but don't touch reserved binding slot
VkVertexInputBindingDescription GenerateReservedVBBindingDesc(uint32_t vertexFormat);

// By default vertex attributes acquire data from reserved vertex buffer binding slot
std::vector<VkVertexInputAttributeDescription> GenerateReservedVBAttribDesc(uint32_t vertexFormat, uint32_t vertexFormatInMem);

void TransferBytesToVector(std::vector<uint8_t>& vec, const void* pData, uint32_t offset, uint32_t numBytes);