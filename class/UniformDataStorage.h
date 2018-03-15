#pragma once

#include "../Maths/Matrix.h"
#include "../Base/Base.h"

class DescriptorSet;
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
	uint32_t	offset;
}UniformVar;

typedef struct _UniformVarList
{
	MaterialVariableType		type;
	std::string					name;
	std::vector<UniformVar>		vars;
}UniformVarList;

class UniformDataStorage : public SelfRefBase<UniformDataStorage>
{
protected:
	static const uint32_t MAXIMUM_OBJECTS = 1024;

public:
	enum UniformType
	{
		GlobalVariable,
		PerFrameVariable,
		PerObjectVariable,
		PerObjectMaterialVariable,
		GBufferAttachments,
		UniformTypeCount
	};

public:
	bool Init(const std::shared_ptr<UniformDataStorage>& pSelf, uint32_t numBytes, bool perObject);

public:
	uint32_t GetFrameOffset() const { return m_frameOffset; }
	void SyncBufferData();
	std::shared_ptr<Buffer> GetBuffer() const;
	virtual std::vector<UniformVarList> PrepareUniformVarList() = 0;
	virtual void SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t reservedIndex = 0) const = 0;

protected:
	virtual void SyncBufferDataInternal() = 0;
	virtual void SetDirty();

protected:
	std::shared_ptr<UniformBuffer>			m_pUniformBuffer;
	std::shared_ptr<ShaderStorageBuffer>	m_pShaderStorageBuffer;
	uint32_t								m_pendingSyncCount;
	uint32_t								m_frameOffset;
};