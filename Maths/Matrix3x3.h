#pragma once

template <typename T>
class Vector3;

template <typename T>
class Matrix3x3
{
public:
	Matrix3x3();
	Matrix3x3(T _00, T _01, T _02,
			  T _10, T _11, T _12,
			  T _20, T _21, T _22);
	Matrix3x3(const Vector3<T>& c0, const Vector3<T>& c1, const Vector3<T>& c2);
	Matrix3x3(const T* pData);
	Matrix3x3(const Matrix3x3<T>& m);

	bool IsIdentity() const;

	Vector3<T>& operator[](unsigned int index);
	const Vector3<T>& operator[](unsigned int index) const;

	Matrix3x3<T>& operator *= (const Matrix3x3<T>& m);
	const Matrix3x3<T> operator * (const Matrix3x3<T>& m) const;
	const Vector3<T> operator * (const Vector3<T>& v) const;

	Matrix3x3& Transpose();
	Matrix3x3& Inverse();

	T Determinant() const;

	static Matrix3x3<T> Rotation(T rotation, const Vector3<T>& v);
	static Matrix3x3<T> EulerAngle(T rotationX, T rotationY, T rotationZ);
public:
	union
	{
		Vector3<T> c[3];
		struct
		{
			T c00, c01, c02;
			T c10, c11, c12;
			T c20, c21, c22;
		};
		struct
		{
			T x0, x1, x2;
			T y0, y1, y2;
			T z0, z1, z2;
		};
	};
};

#include "Matrix3x3.inl"