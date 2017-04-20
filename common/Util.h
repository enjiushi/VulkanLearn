#pragma once
#include <assert.h>
#include <GL\glew.h>
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