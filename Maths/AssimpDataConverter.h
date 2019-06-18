#pragma once

#include "Matrix.h"
#include "DualQuaternion.h"
#include "scene.h"

class AssimpDataConverter
{
public:
	static Vector3f AcquireVector3(const aiVector3D& assimpVector3);
	static Quaternionf AcquireQuaternion(const aiQuaternion& assimpQuaternion);
	static Matrix3f AcquireMatrix3(const aiMatrix3x3& assimpMatrix3x3);
	static Matrix3f AcquireRotationMatrix(const aiMatrix4x4& assimpMatrix4x4);
	static Quaternionf AcquireRotationQuaternion(const aiMatrix4x4& assimpMatrix4x4);
	static Vector3f AcquireTranslationVector(const aiMatrix4x4& assimpMatrix4x4);
	static Matrix4f AcquireMatrix(const aiMatrix4x4& assimpMatrix4x4);
	static DualQuaternionf AcquireDualQuaternion(const aiMatrix4x4& assimpMatrix4x4);
};