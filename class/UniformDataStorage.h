#pragma once

#include "../Maths/Matrix.h"
#include "../Base/Base.h"

class Buffer;
class UniformBuffer;
class ShaderStorageBuffer;

enum MaterialVariableType;

enum UBOType
{
	OneUnit,
	Vec2Unit,
	Vec3Unit,
	Vec4Unit,
	Mat3Unit,
	Mat4Unit,
	UBOTypeCount
};

typedef struct _UniformVar
{
	UBOType		type;
	std::string name;
}UniformVar;

typedef struct _UniformVarList
{
	MaterialVariableType		type;
	std::string					name;
	std::vector<UniformVar>		vars;
}UniformVarList;

class UniformDataStorage : public SelfRefBase<UniformDataStorage>
{
public:
	enum UniformType
	{
		GlobalVariable,
		PerFrameVariable,
		PerObjectVariable,
		PerObjectMaterialVariable,
		UniformTypeCount
	};

public:
	bool Init(const std::shared_ptr<UniformDataStorage>& pSelf, uint32_t numBytes);

public:
	uint32_t GetFrameOffset() const { return m_frameOffset; }
	void SyncBufferData();
	std::shared_ptr<Buffer> GetBuffer();
	virtual UniformVarList PrepareUniformVarList() = 0;

protected:
	virtual void SyncBufferDataInternal() = 0;
	virtual void SetDirty();

protected:
	std::shared_ptr<UniformBuffer>			m_pUniformBuffer;
	std::shared_ptr<ShaderStorageBuffer>	m_pShaderStorageBuffer;
	uint32_t								m_pendingSyncCount;
	uint32_t								m_frameOffset;
};