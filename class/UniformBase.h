#pragma once

#include "../Maths/Matrix.h"
#include "../Base/Base.h"

class DescriptorSet;

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

class UniformBase : public SelfRefBase<UniformBase>
{
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
	bool Init(const std::shared_ptr<UniformBase>& pSelf);

public:
	virtual std::vector<UniformVarList> PrepareUniformVarList() = 0;
	virtual void SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t reservedIndex = 0) const = 0;
};