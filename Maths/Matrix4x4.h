#pragma once

template <typename T>
class Vector3;

template <typename T>
class Vector4;

template <typename T>
class Matrix3x3;

template <typename T>
class Matrix4x4
{
public:
	Matrix4x4();
	Matrix4x4(T _00, T _01, T _02, T _03,
			  T _10, T _11, T _12, T _13,
			  T _20, T _21, T _22, T _23,
			  T _30, T _31, T _32, T _33);
	Matrix4x4(const Vector4<T>& c0, const Vector4<T>& c1, const Vector4<T>& c2, const Vector4<T>& c3);
	Matrix4x4(const T* pData);
	Matrix4x4(const Matrix4x4<T>& m);
	Matrix4x4(const Matrix3x3<T>& m);
	Matrix4x4(const Matrix3x3<T>& m, const Vector3<T>& v);
	Matrix4x4(const Vector4<T>& v);

	bool IsIdentity() const;

	Vector4<T>& operator[](unsigned int index);
	const Vector4<T>& operator[](unsigned int index) const;

	Matrix4x4<T>& operator *= (const Matrix4x4<T>& m);
	Matrix4x4<T>& operator *= (const Matrix3x3<T>& m);
	Matrix4x4<T>& operator += (const Matrix4x4<T>& m);
	Matrix4x4<T>& operator -= (const Matrix4x4<T>& m);

	Matrix4x4<T>& operator *= (T s);
	Matrix4x4<T>& operator += (T s);
	Matrix4x4<T>& operator -= (T s);

	const Matrix4x4<T> operator * (const Matrix4x4<T>& m) const;
	const Matrix4x4<T> operator * (const Matrix3x3<T>& m) const;
	const Matrix4x4<T> operator + (const Matrix4x4<T>& m) const;
	const Matrix4x4<T> operator - (const Matrix4x4<T>& m) const;

	const Matrix4x4<T> operator * (T s) const;
	const Matrix4x4<T> operator + (T s) const;
	const Matrix4x4<T> operator - (T s) const;

	const Vector4<T> operator * (const Vector4<T>& v) const;

	Matrix4x4<T>& operator = (const Matrix3x3<T>& m);

	Matrix4x4& Transpose();
	Matrix4x4& Inverse();

	static Matrix4x4<T> Rotation(T rotation, const Vector3<T>& v);
	static Matrix4x4<T> Rotation(T rotation, const Vector4<T>& v);
	static Matrix4x4<T> EulerAngle(T rotationX, T rotationY, T rotationZ);

	Matrix3x3<T> RotationMatrix() const;
	Vector3<T> TranslationVector() const;
	T Determinant() const;

	Vector3<T> TransformAsVector(const Vector3<T>& v) const;
	Vector3<T> TransformAsPoint(const Vector3<T>& v) const;

public:
	union
	{
		Vector4<T> c[4];
		struct
		{
			T c00, c01, c02, c03;
			T c10, c11, c12, c13;
			T c20, c21, c22, c23;
			T c30, c31, c32, c33;
		};
		struct
		{
			T x0, x1, x2, x3;
			T y0, y1, y2, y3;
			T z0, z1, z2, z3;
			T w0, w1, w2, w3;
		};
	};
};

#include "Matrix4x4.inl"