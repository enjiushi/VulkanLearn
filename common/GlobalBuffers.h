#pragma once
#include "../vulkan/UniformBuffer.h"
#include "Singleton.h"
#include "../Maths/Matrix.h"

typedef struct _PerObjectVariables
{
	Matrix4f modelTransform;
}PerObjectVariables;

class GlobalBuffers : Singleton<GlobalBuffers>
{
	static const uint32_t MAXIMUM_OBJECTS = 1024;

public:
	std::shared_ptr<UniformBuffer> GetObjectsUniformBuffer() { return m_pObjectsUniformBuffer; }

protected:
	bool Init() override;

protected:
	std::shared_ptr<UniformBuffer> m_pObjectsUniformBuffer;
};