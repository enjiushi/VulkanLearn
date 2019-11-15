#pragma once

#include "../Maths/Matrix.h"
#include "../Base/Base.h"
#include "PerFrameDataStorage.h"
#include "IMaterialUniformOperator.h"

class BufferBase;

enum MaterialVariableType;

class UniformDataStorage : public PerFrameDataStorage, public IMaterialUniformOperator
{
protected:
	bool Init(const std::shared_ptr<UniformDataStorage>& pSelf, uint32_t numBytes, StorageType storageType);
};