#pragma once
#include "Matrix3x3.h"
#include "Vector.h"
#include <algorithm>

template <typename T>
Matrix3x3<T>::Matrix3x3()
{
	c[0].x = 1; c[0].y = 0; c[0].z = 0;
	c[1].x = 0; c[1].y = 1; c[1].z = 0;
	c[2].x = 0; c[2].y = 0; c[2].z = 1;
}

template <typename T>
Matrix3x3<T>::Matrix3x3(T _00, T _01, T _02,
						T _10, T _11, T _12,
						T _20, T _21, T _22)
{
	c[0].x = _00; c[0].y = _01; c[0].z = _02;
	c[1].x = _10; c[1].y = _11; c[1].z = _12;
	c[2].x = _20; c[2].y = _21; c[2].z = _22;
}

template <typename T>
Matrix3x3<T>::Matrix3x3(const T* pData)
{
	c[0].x = pData[0]; c[0].y = pData[1]; c[0].z = pData[2];
	c[1].x = pData[3]; c[1].y = pData[4]; c[1].z = pData[5];
	c[2].x = pData[6]; c[2].y = pData[7]; c[2].z = pData[8];
}

template <typename T>
Matrix3x3<T>::Matrix3x3(const Matrix3x3<T>& m)
{
	c[0] = m.c[0];
	c[1] = m.c[1];
	c[2] = m.c[2];
}

template <typename T>
Matrix3x3<T>::Matrix3x3(const Vector3<T>& c0, const Vector3<T>& c1, const Vector3<T>& c2)
{
	c[0] = c0;
	c[1] = c1;
	c[2] = c2;
}

template <typename T>
bool Matrix3x3<T>::IsIdentity() const
{
	const static T eps = 10e-3f;

	bool ret = (c[0].x > 1.0 - eps) && (c[0].x < 1.0 + eps)
		&& (c[0].y > -eps) && (c[0].y < eps)
		&& (c[0].z > -eps) && (c[0].z < eps)
		&& (c[1].x > -eps) && (c[1].x < eps)
		&& (c[1].y > 1.0 - eps) && (c[1].y < 1.0 + eps)
		&& (c[1].z > -eps) && (c[1].z < eps)
		&& (c[2].x > -eps) && (c[2].x < eps)
		&& (c[2].y > -eps) && (c[2].y < eps)
		&& (c[2].z > 1.0 - eps) && (c[2].z < 1.0 + eps);

	return ret;
}

template <typename T>
Vector3<T>& Matrix3x3<T>::operator[](unsigned int index)
{
	return c[index];
}

template <typename T>
const Vector3<T>& Matrix3x3<T>::operator[](unsigned int index) const
{
	return c[index];
}

template <typename T>
Matrix3x3<T>& Matrix3x3<T>::operator*=(const Matrix3x3<T>& m)
{
	*this = Matrix3x3<T>(
		c00 * m.c00 + c10 * m.c01 + c20 * m.c02,
		c01 * m.c00 + c11 * m.c01 + c21 * m.c02,
		c02 * m.c00 + c12 * m.c01 + c22 * m.c02,

		c00 * m.c10 + c10 * m.c11 + c20 * m.c12,
		c01 * m.c10 + c11 * m.c11 + c21 * m.c12,
		c02 * m.c10 + c12 * m.c11 + c22 * m.c12,

		c00 * m.c20 + c10 * m.c21 + c20 * m.c22,
		c01 * m.c20 + c11 * m.c21 + c21 * m.c22,
		c02 * m.c20 + c12 * m.c21 + c22 * m.c22
	);

	return *this;
}

template <typename T>
const Matrix3x3<T> Matrix3x3<T>::operator*(const Matrix3x3<T>& m) const
{
	Matrix3x3<T> ret_m = *this;
	ret_m *= m;
	return ret_m;
}

template <typename T>
const Vector3<T> Matrix3x3<T>::operator * (const Vector3<T>& v) const
{
	Vector3<T> ret_v;
	ret_v.x = x0 * v.x + y0 * v.y + z0 * v.z;
	ret_v.y = x1 * v.x + y1 * v.y + z1 * v.z;
	ret_v.z = x2 * v.x + y2 * v.y + z2 * v.z;
	return ret_v;
}

template <typename T>
Matrix3x3<T>& Matrix3x3<T>::Transpose()
{
	std::swap<T>(c01, c10);
	std::swap<T>(c02, c20);
	std::swap<T>(c12, c21);
	return *this;
}

template <typename T>
Matrix3x3<T>& Matrix3x3<T>::Inverse()
{
	T det = Determinant();
	if (det == static_cast<T>(0.0))
	{
		const T nan = std::numeric_limits<T>::quiet_NaN();
		*this = Matrix3x3<T>(nan, nan, nan, nan, nan, nan, nan, nan, nan);

		return *this;
	}

	T invdet = static_cast<T>(1.0) / det;

	Matrix3x3<T> m;
	m.c00 = invdet  * (c11 * c22 - c12 * c21);
	m.c01 = -invdet * (c01 * c22 - c02 * c21);
	m.c02 = invdet  * (c01 * c12 - c02 * c11);
	m.c10 = -invdet * (c10 * c22 - c12 * c20);
	m.c11 = invdet  * (c00 * c22 - c02 * c20);
	m.c12 = -invdet * (c00 * c12 - c02 * c10);
	m.c20 = invdet  * (c10 * c21 - c11 * c20);
	m.c21 = -invdet * (c00 * c21 - c01 * c20);
	m.c22 = invdet  * (c00 * c11 - c01 * c10);
	*this = m;

	return *this;
}

template <typename T>
T Matrix3x3<T>::Determinant() const
{
	return c00 * c11 * c22 + c01 * c12 * c20 + c02 * c10 * c21 - c02 * c11 * c20 - c01 * c10 * c22 - c00 * c12 * c21;
}

template <typename T>
Matrix3x3<T> Matrix3x3<T>::Rotation(T rotation, const Vector3<T>& v)
{
	T c = std::cos(rotation), s = std::sin(rotation), t = 1 - c;
	T x = v.x, y = v.y, z = v.z;

	Matrix3x3<T> out;

	out.c00 = t*x*x + c;   out.c10 = t*x*y - s*z; out.c20 = t*x*z + s*y;
	out.c01 = t*x*y + s*z; out.c11 = t*y*y + c;   out.c21 = t*y*z - s*x;
	out.c02 = t*x*z - s*y; out.c12 = t*y*z + s*x; out.c22 = t*z*z + c;

	return out;
}

template <typename T>
Matrix3x3<T> Matrix3x3<T>::EulerAngle(T rotationX, T rotationY, T rotationZ)
{
	Matrix3x3<T> m;

	//http://planning.cs.uiuc.edu/node102.html
	//we use yaw-pitch-roll order, i.e. Rz*Ry*Rx * v
	T cos_rol = std::cos(rotationX);
	T sin_rol = std::sin(rotationX);
	T cos_pit = std::cos(rotationY);
	T sin_pit = std::sin(rotationY);
	T cos_yaw = std::cos(rotationZ);
	T sin_yaw = std::sin(rotationZ);

	m.c00 = cos_yaw * cos_pit;
	m.c01 = sin_yaw * cos_pit;
	m.c02 = -sin_pit;

	T srsp = sin_pit * sin_rol;
	T crsp = sin_pit * cos_rol;

	m.c10 = srsp * cos_yaw - sin_yaw * cos_rol;
	m.c11 = srsp * sin_yaw + cos_yaw * cos_rol;
	m.c12 = cos_pit * sin_rol;

	m.c20 = crsp * cos_yaw + sin_yaw * sin_rol;
	m.c21 = crsp * sin_yaw - cos_yaw * sin_rol;
	m.c22 = cos_pit * cos_rol;

	return m;
}