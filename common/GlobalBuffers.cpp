#pragma once
#include "GlobalBuffers.h"

bool GlobalBuffers::Init()
{
	m_pObjectsUniformBuffer = UniformBuffer::Create(GetDevice(), MAXIMUM_OBJECTS * sizeof(Matrix4f));
	return true;
}