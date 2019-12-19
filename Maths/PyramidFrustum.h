#pragma once

template <typename T>
class Vector3;

template <typename T>
class Matrix3x3;

template <typename T>
class Matrix4x4;

template <typename T>
class Plane;

template<typename T>
class PyramidFrustum
{
public:
	enum FrustumFace
	{
		FrustumFace_LEFT,
		FrustumFace_RIGHT,
		FrustumFace_BOTTOM,
		FrustumFace_TOP,
		FrustumFace_COUNT,
	};

public:
	PyramidFrustum() = default;

	PyramidFrustum(const Vector3<T>& head, const Vector3<T>& bottomLeft, const Vector3<T>& bottomRight, const Vector3<T>& topLeft, const Vector3<T>& topRight);

	PyramidFrustum(const Vector3<T>& head, const Vector3<T>& lookAt, T fovv, T aspect);

public:
	bool Contain(const Vector3<T>& p) const;

	// Matrix rotation part should be orthogonal
	void Transform(const Matrix3x3<T>& matrix);
	void Transform(const Matrix4x4<T>& matrix);

public:
	Plane<T>	planes[FrustumFace_COUNT];
	Vector3<T>	head;
};

#include "PyramidFrustum.inl"

typedef PyramidFrustum<float>	PyramidFrustumf;
typedef PyramidFrustum<double>	PyramidFrustumd;