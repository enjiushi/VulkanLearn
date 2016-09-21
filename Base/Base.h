#pragma once

#include "RefCounted.h"
#include "GL\glew.h"

class Base : public RefCounted
{
public:
	Base() {}
	virtual ~Base() = 0 {}

	virtual GLboolean Init() { return GL_TRUE; }
};