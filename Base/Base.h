#pragma once

#include "../common/AutoPTR.h"

class Base : public RefCounted
{
public:
	Base() {}
	virtual ~Base() = 0 {}

	virtual bool Init() { return true; }
};