#pragma once
#include "Matrix4x4.h"
#include "Vector.h"
#include "Matrix3x3.inl"
#include <algorithm>

template <typename T>
Matrix4x4<T>::Matrix4x4()
{
	c[0].x = 1; c[0].y = 0; c[0].z = 0, c[0].w = 0;
	c[1].x = 0; c[1].y = 1; c[1].z = 0; c[1].w = 0;
	c[2].x = 0; c[2].y = 0; c[2].z = 1; c[2].w = 0;
	c[3].x = 0; c[3].y = 0; c[3].z = 0; c[3].w = 1;
}

template <typename T>
Matrix4x4<T>::Matrix4x4(T _00, T _01, T _02, T _03,
						T _10, T _11, T _12, T _13,
						T _20, T _21, T _22, T _23,
						T _30, T _31, T _32, T _33)
{
	c[0].x = _00; c[0].y = _01; c[0].z = _02; c[0].w = _03;
	c[1].x = _10; c[1].y = _11; c[1].z = _12; c[1].w = _13;
	c[2].x = _20; c[2].y = _21; c[2].z = _22; c[2].w = _23;
	c[3].x = _30; c[3].y = _31; c[3].z = _32; c[3].w = _33;
}

template <typename T>
Matrix4x4<T>::Matrix4x4(const T* pData)
{
	c[0].x = pData[0]; c[0].y = pData[1]; c[0].z = pData[2]; c[0].w = pData[3];
	c[1].x = pData[4]; c[1].y = pData[5]; c[1].z = pData[6]; c[1].w = pData[7];
	c[2].x = pData[8]; c[2].y = pData[9]; c[2].z = pData[10]; c[2].w = pData[11];
	c[3].x = pData[12]; c[3].y = pData[13]; c[3].z = pData[14]; c[3].w = pData[15];
}

template <typename T>
Matrix4x4<T>::Matrix4x4(const Matrix4x4<T>& m)
{
	c[0] = m.c[0];
	c[1] = m.c[1];
	c[2] = m.c[2];
	c[3] = m.c[3];
}

template <typename T>
Matrix4x4<T>::Matrix4x4(const Matrix3x3<T>& m)
{
	c[0].x = m.c[0].x; c[0].y = m.c[0].y; c[0].z = m.c[0].z; c[0].w = 0;
	c[1].x = m.c[1].x; c[1].y = m.c[1].y; c[1].z = m.c[1].z; c[1].w = 0;
	c[2].x = m.c[2].x; c[2].y = m.c[2].y; c[2].z = m.c[2].z; c[2].w = 0;
	c[3].x = 0; c[3].y = 0; c[3].z = 0; c[3].w = 1;
}

template <typename T>
Matrix4x4<T>::Matrix4x4(const Matrix3x3<T>& m, const Vector3<T>& v)
{
	c[0].x = m.c[0].x; c[0].y = m.c[0].y; c[0].z = m.c[0].z; c[0].w = 0;
	c[1].x = m.c[1].x; c[1].y = m.c[1].y; c[1].z = m.c[1].z; c[1].w = 0;
	c[2].x = m.c[2].x; c[2].y = m.c[2].y; c[2].z = m.c[2].z; c[2].w = 0;
	c[3].x = v.x; c[3].y = v.y; c[3].z = v.z; c[3].w = 1;
}

template <typename T>
Matrix4x4<T>::Matrix4x4(const Vector4<T>& c0, const Vector4<T>& c1, const Vector4<T>& c2, const Vector4<T>& c3)
{
	c[0] = c0;
	c[1] = c1;
	c[2] = c2;
	c[3] = c3;
}

template <typename T>
Matrix4x4<T>::Matrix4x4(const Vector4<T>& v)
{
	c[0].x = v.x; c[0].y = 0; c[0].z = 0, c[0].w = 0;
	c[1].x = 0; c[1].y = v.y; c[1].z = 0; c[1].w = 0;
	c[2].x = 0; c[2].y = 0; c[2].z = v.z; c[2].w = 0;
	c[3].x = 0; c[3].y = 0; c[3].z = 0; c[3].w = v.w;
}

template <typename T>
bool Matrix4x4<T>::IsIdentity() const
{
	const static T eps = 10e-3f;

	bool ret = (c[0].x > 1.0 - eps) && (c[0].x < 1.0 + eps)
		&& (c[0].y > -eps) && (c[0].y < eps)
		&& (c[0].z > -eps) && (c[0].z < eps)
		&& (c[0].w > -eps) && (c[0].w < eps)
		&& (c[1].x > -eps) && (c[1].x < eps)
		&& (c[1].y > 1.0 - eps) && (c[1].y < 1.0 + eps)
		&& (c[1].z > -eps) && (c[1].z < eps)
		&& (c[1].w > -eps) && (c[1].w < eps)
		&& (c[2].x > -eps) && (c[2].x < eps)
		&& (c[2].y > -eps) && (c[2].y < eps)
		&& (c[2].z > 1.0 - eps) && (c[2].z < 1.0 + eps)
		&& (c[2].w > -eps) && (c[3].w < eps)
		&& (c[3].x > -eps) && (c[3].x < eps)
		&& (c[3].y > -eps) && (c[3].y < eps)
		&& (c[3].z > -eps) && (c[3].z < eps)
		&& (c[3].w > 1.0 - eps) && (c[3].w < 1.0 + eps);

	return ret;
}

template <typename T>
Vector4<T>& Matrix4x4<T>::operator[](unsigned int index)
{
	return c[index];
}

template <typename T>
const Vector4<T>& Matrix4x4<T>::operator[](unsigned int index) const
{
	return c[index];
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::operator*=(const Matrix4x4<T>& m)
{
	*this = Matrix4x4<T>(
		c00 * m.c00 + c10 * m.c01 + c20 * m.c02 + c30 * m.c03,
		c01 * m.c00 + c11 * m.c01 + c21 * m.c02 + c31 * m.c03,
		c02 * m.c00 + c12 * m.c01 + c22 * m.c02 + c32 * m.c03,
		c03 * m.c00 + c13 * m.c01 + c23 * m.c02 + c33 * m.c03,

		c00 * m.c10 + c10 * m.c11 + c20 * m.c12 + c30 * m.c13,
		c01 * m.c10 + c11 * m.c11 + c21 * m.c12 + c31 * m.c13,
		c02 * m.c10 + c12 * m.c11 + c22 * m.c12 + c32 * m.c13,
		c03 * m.c10 + c13 * m.c11 + c23 * m.c12 + c33 * m.c13,

		c00 * m.c20 + c10 * m.c21 + c20 * m.c22 + c30 * m.c23,
		c01 * m.c20 + c11 * m.c21 + c21 * m.c22 + c31 * m.c23,
		c02 * m.c20 + c12 * m.c21 + c22 * m.c22 + c32 * m.c23,
		c03 * m.c20 + c13 * m.c21 + c23 * m.c22 + c33 * m.c23,

		c00 * m.c30 + c10 * m.c31 + c20 * m.c32 + c30 * m.c33,
		c01 * m.c30 + c11 * m.c31 + c21 * m.c32 + c31 * m.c33,
		c02 * m.c30 + c12 * m.c31 + c22 * m.c32 + c32 * m.c33,
		c03 * m.c30 + c13 * m.c31 + c23 * m.c32 + c33 * m.c33
	);

	return *this;
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::operator*=(const Matrix3x3<T>& m)
{
	Matrix4x4<T> m_4x4(m, Vector3<T>(0, 0, 0));
	*this *= m_4x4;

	return *this;
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::operator += (const Matrix4x4<T>& m)
{
	*this = Matrix4x4<T>(
		c[0] + m.c[0],
		c[1] + m.c[1],
		c[2] + m.c[2],
		c[3] + m.c[3]
		);

	return *this;
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::operator -= (const Matrix4x4<T>& m)
{
	*this = Matrix4x4<T>(
		c[0] - m.c[0],
		c[1] - m.c[1],
		c[2] - m.c[2],
		c[3] - m.c[3]
		);

	return *this;
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::operator *= (T s)
{
	*this = Matrix4x4<T>(
		c[0] * s,
		c[1] * s,
		c[2] * s,
		c[3] * s
		);

	return *this;
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::operator += (T s)
{
	*this = Matrix4x4<T>(
		c[0] + s,
		c[1] + s,
		c[2] + s,
		c[3] + s
		);

	return *this;
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::operator -= (T s)
{
	*this = Matrix4x4<T>(
		c[0] - s,
		c[1] - s,
		c[2] - s,
		c[3] - s
		);

	return *this;
}

template <typename T>
const Matrix4x4<T> Matrix4x4<T>::operator*(const Matrix4x4<T>& m) const
{
	Matrix4x4<T> ret = *this;
	ret *= m;
	return ret;
}

template <typename T>
const Matrix4x4<T> Matrix4x4<T>::operator*(const Matrix3x3<T>& m) const
{
	Matrix4x4<T> m_4x4(m, Vector3<T>(0, 0, 0));
	Matrix4x4<T> ret;
	ret *= m_4x4;
	return ret;
}

template <typename T>
const Vector4<T> Matrix4x4<T>::operator*(const Vector4<T>& v) const
{
	Vector4<T> ret;
	ret.x = x0 * v.x + y0 * v.y + z0 * v.z + w0 * v.w;
	ret.y = x1 * v.x + y1 * v.y + z1 * v.z + w1 * v.w;
	ret.z = x2 * v.x + y2 * v.y + z2 * v.z + w2 * v.w;
	ret.w = x3 * v.x + y3 * v.y + z3 * v.z + w3 * v.w;
	return ret;
}

template <typename T>
const Matrix4x4<T> Matrix4x4<T>::operator + (const Matrix4x4<T>& m) const
{
	Matrix4x4<T> ret = *this;
	ret += m;
	return ret;
}

template <typename T>
const Matrix4x4<T> Matrix4x4<T>::operator - (const Matrix4x4<T>& m) const
{
	Matrix4x4<T> ret = *this;
	ret -= s;
	return ret;
}

template <typename T>
const Matrix4x4<T> Matrix4x4<T>::operator * (T s) const
{
	Matrix4x4<T> ret = *this;
	ret *= s;
	return ret;
}

template <typename T>
const Matrix4x4<T> Matrix4x4<T>::operator + (T s) const
{
	Matrix4x4<T> ret = *this;
	ret += s;
	return ret;
}

template <typename T>
const Matrix4x4<T> Matrix4x4<T>::operator - (T s) const
{
	Matrix4x4<T> ret = *this;
	ret -= s;
	return ret;
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::operator = (const Matrix3x3<T>& m)
{
	c[0].x = m.c[0].x; c[0].y = m.c[0].y; c[0].z = m.c[0].z; c[0].w = 0;
	c[1].x = m.c[1].x; c[1].y = m.c[1].y; c[1].z = m.c[1].z; c[1].w = 0;
	c[2].x = m.c[2].x; c[2].y = m.c[2].y; c[2].z = m.c[2].z; c[2].w = 0;
	return *this;
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::Transpose()
{
	std::swap<T>(c01, c10);
	std::swap<T>(c02, c20);
	std::swap<T>(c12, c21);
	std::swap<T>(c03, c30);
	std::swap<T>(c13, c31);
	std::swap<T>(c23, c32);
	return *this;
}

template <typename T>
Matrix4x4<T>& Matrix4x4<T>::Inverse()
{
	const T det = Determinant();
	if (det == static_cast<T>(0.0))
	{
		const T nan = std::numeric_limits<T>::quiet_NaN();
		*this = Matrix4x4<T>(
			nan, nan, nan, nan,
			nan, nan, nan, nan,
			nan, nan, nan, nan,
			nan, nan, nan, nan);

		return *this;
	}

	const T invdet = static_cast<T>(1.0) / det;

	Matrix4x4<T> res;

	res.x0 = invdet  * (y1 * (z2 * w3 - w2 * z3) + z1 * (w2 * y3 - y2 * w3) + w1 * (y2 * z3 - z2 * y3));
	res.x1 = -invdet * (x1 * (z2 * w3 - w2 * z3) + z1 * (w2 * x3 - x2 * w3) + w1 * (x2 * z3 - z2 * x3));
	res.x2 = invdet  * (x1 * (y2 * w3 - w2 * y3) + y1 * (w2 * x3 - x2 * w3) + w1 * (x2 * y3 - y2 * x3));
	res.x3 = -invdet * (x1 * (y2 * z3 - z2 * y3) + y1 * (z2 * x3 - x2 * z3) + z1 * (x2 * y3 - y2 * x3));

	res.y0 = -invdet * (y0 * (z2 * w3 - w2 * z3) + z0 * (w2 * y3 - y2 * w3) + w0 * (y2 * z3 - z2 * y3));
	res.y1 = invdet  * (x0 * (z2 * w3 - w2 * z3) + z0 * (w2 * x3 - x2 * w3) + w0 * (x2 * z3 - z2 * x3));
	res.y2 = -invdet * (x0 * (y2 * w3 - w2 * y3) + y0 * (w2 * x3 - x2 * w3) + w0 * (x2 * y3 - y2 * x3));
	res.y3 = invdet  * (x0 * (y2 * z3 - z2 * y3) + y0 * (z2 * x3 - x2 * z3) + z0 * (x2 * y3 - y2 * x3));

	res.z0 = invdet  * (y0 * (z1 * w3 - w1 * z3) + z0 * (w1 * y3 - y1 * w3) + w0 * (y1 * z3 - z1 * y3));
	res.z1 = -invdet * (x0 * (z1 * w3 - w1 * z3) + z0 * (w1 * x3 - x1 * w3) + w0 * (x1 * z3 - z1 * x3));
	res.z2 = invdet  * (x0 * (y1 * w3 - w1 * y3) + y0 * (w1 * x3 - x1 * w3) + w0 * (x1 * y3 - y1 * x3));
	res.z3 = -invdet * (x0 * (y1 * z3 - z1 * y3) + y0 * (z1 * x3 - x1 * z3) + z0 * (x1 * y3 - y1 * x3));

	res.w0 = -invdet * (y0 * (z1 * w2 - w1 * z2) + z0 * (w1 * y2 - y1 * w2) + w0 * (y1 * z2 - z1 * y2));
	res.w1 = invdet  * (x0 * (z1 * w2 - w1 * z2) + z0 * (w1 * x2 - x1 * w2) + w0 * (x1 * z2 - z1 * x2));
	res.w2 = -invdet * (x0 * (y1 * w2 - w1 * y2) + y0 * (w1 * x2 - x1 * w2) + w0 * (x1 * y2 - y1 * x2));
	res.w3 = invdet  * (x0 * (y1 * z2 - z1 * y2) + y0 * (z1 * x2 - x1 * z2) + z0 * (x1 * y2 - y1 * x2));

	*this = res;

	return *this;
}

template <typename T>
T Matrix4x4<T>::Determinant() const
{
	return x0*y1*z2*w3 - x0*y1*w2*z3 + x0*z1*w2*y3 - x0*z1*y2*w3
		+ x0*w1*y2*z3 - x0*w1*z2*y3 - y0*z1*w2*x3 + y0*z1*x2*w3
		- y0*w1*x2*z3 + y0*w1*z2*x3 - y0*x1*z2*w3 + y0*x1*w2*z3
		+ z0*w1*x2*y3 - z0*w1*y2*x3 + z0*x1*y2*w3 - z0*x1*w2*y3
		+ z0*y1*w2*x3 - z0*y1*x2*w3 - w0*x1*y2*z3 + w0*x1*z2*y3
		- w0*y1*z2*x3 + w0*y1*x2*z3 - w0*z1*x2*y3 + w0*z1*y2*x3;
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::Rotation(T rotation, const Vector3<T>& v)
{
	Matrix3x3<T> ret = Matrix3x3<T>::Rotation(rotation, v);
	return Matrix4x4<T>(ret);
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::Rotation(T rotation, const Vector4<T>& v)
{
	return Rotation(v.DivHomograph());
}

template <typename T>
Matrix4x4<T> Matrix4x4<T>::EulerAngle(T rotationX, T rotationY, T rotationZ)
{
	Matrix3x3<T> ret = Matrix3x3<T>::EulerAngle(rotationX, rotationY, rotationZ);
	return Matrix4x4<T>(ret);
}

template <typename T>
Matrix3x3<T> Matrix4x4<T>::RotationMatrix() const
{
	return Matrix3x3<T>(c[0].xyz(), c[1].xyz(), c[2].xyz());
}

template <typename T>
Vector3<T> Matrix4x4<T>::TranslationVector() const
{
	return c[3].xyz();
}

template <typename T>
Vector3<T> Matrix4x4<T>::TransformAsVector(const Vector3<T>& v) const
{
	return (*this * Vector4<T>(v, 0.0f)).xyz();
}

template <typename T>
Vector3<T> Matrix4x4<T>::TransformAsPoint(const Vector3<T>& v) const
{
	return (*this * Vector4<T>(v, 1.0f)).xyz();
}