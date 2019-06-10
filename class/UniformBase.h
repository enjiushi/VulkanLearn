#pragma once

#include "../Maths/Matrix.h"
#include "../Base/Base.h"

class DescriptorSetLayout;
class DescriptorSet;

enum MaterialVariableType;

enum UBOType
{
	OneUnit,
	Vec2Unit,
	Vec3Unit,
	Vec4Unit,
	Mat2x4Unit,
	Mat3Unit,
	Mat4Unit,
	UBOTypeCount
};

typedef struct _UniformVar
{
	UBOType		type;
	std::string name;
	uint32_t	offset;
	uint32_t	count = 1;
}UniformVar;

typedef struct _UniformVarList
{
	MaterialVariableType		type;
	std::string					name;
	std::vector<UniformVar>		vars;
	uint32_t					count = 1;
}UniformVarList;

class UniformBase : public SelfRefBase<UniformBase>
{
public:
	bool Init(const std::shared_ptr<UniformBase>& pSelf);

public:
	virtual std::vector<UniformVarList> PrepareUniformVarList() const = 0;
	virtual uint32_t SetupDescriptorSet(const std::shared_ptr<DescriptorSet>& pDescriptorSet, uint32_t bindingIndex) const = 0;		// Returns next available binding index
};