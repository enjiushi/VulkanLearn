#include "AssimpDataConverter.h"

Vector3f AssimpDataConverter::AcquireVector3(const aiVector3D& assimpVector3)
{
	return
	{
		assimpVector3.x,
		assimpVector3.y,
		assimpVector3.z
	};
}

Quaternionf AssimpDataConverter::AcquireQuaternion(const aiQuaternion& assimpQuaternion)
{
	return
	{
		assimpQuaternion.x,
		assimpQuaternion.y,
		assimpQuaternion.z,
		assimpQuaternion.w
	};
}

Matrix3f AssimpDataConverter::AcquireMatrix3(const aiMatrix3x3& assimpMatrix3x3)
{
	return
	{
		assimpMatrix3x3.a1, assimpMatrix3x3.b1, assimpMatrix3x3.c1,
		assimpMatrix3x3.a2, assimpMatrix3x3.b2, assimpMatrix3x3.c2,
		assimpMatrix3x3.a3, assimpMatrix3x3.b3, assimpMatrix3x3.c3
	};
}

Matrix3f AssimpDataConverter::AcquireRotationMatrix(const aiMatrix4x4& assimpMatrix4x4)
{
	return 
	{
		assimpMatrix4x4.a1, assimpMatrix4x4.b1, assimpMatrix4x4.c1,
		assimpMatrix4x4.a2, assimpMatrix4x4.b2, assimpMatrix4x4.c2,
		assimpMatrix4x4.a3, assimpMatrix4x4.b3, assimpMatrix4x4.c3
	};
}

Quaternionf AssimpDataConverter::AcquireRotationQuaternion(const aiMatrix4x4& assimpMatrix4x4)
{
	return AcquireRotationMatrix(assimpMatrix4x4).AcquireQuaternion();
}

Vector3f AssimpDataConverter::AcquireTranslationVector(const aiMatrix4x4& assimpMatrix4x4)
{
	return 
	{
		assimpMatrix4x4.a4, assimpMatrix4x4.b4, assimpMatrix4x4.c4
	};
}

Matrix4f AssimpDataConverter::AcquireMatrix(const aiMatrix4x4& assimpMatrix4x4)
{
	return
	{
		assimpMatrix4x4.a1, assimpMatrix4x4.b1, assimpMatrix4x4.c1, 0,
		assimpMatrix4x4.a2, assimpMatrix4x4.b2, assimpMatrix4x4.c2, 0,
		assimpMatrix4x4.a3, assimpMatrix4x4.b3, assimpMatrix4x4.c3, 0,
		assimpMatrix4x4.a4, assimpMatrix4x4.b4, assimpMatrix4x4.c4, 1
	};
}

DualQuaternionf AssimpDataConverter::AcquireDualQuaternion(const aiMatrix4x4& assimpMatrix4x4)
{
	return
	{
		AcquireRotationQuaternion(assimpMatrix4x4),
		AcquireTranslationVector(assimpMatrix4x4)
	};
}