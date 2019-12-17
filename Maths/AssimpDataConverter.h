#pragma once

#include "Matrix.h"
#include "DualQuaternion.h"
#include "scene.h"

class AssimpDataConverter
{
public:
	static Vector3d AcquireVector3(const aiVector3D& assimpVector3);
	static Quaterniond AcquireQuaternion(const aiQuaternion& assimpQuaternion);
	static Matrix3d AcquireMatrix3(const aiMatrix3x3& assimpMatrix3x3);
	static Matrix3d AcquireRotationMatrix(const aiMatrix4x4& assimpMatrix4x4);
	static Quaterniond AcquireRotationQuaternion(const aiMatrix4x4& assimpMatrix4x4);
	static Vector3d AcquireTranslationVector(const aiMatrix4x4& assimpMatrix4x4);
	static Matrix4d AcquireMatrix(const aiMatrix4x4& assimpMatrix4x4);
	static DualQuaterniond AcquireDualQuaternion(const aiMatrix4x4& assimpMatrix4x4);
};