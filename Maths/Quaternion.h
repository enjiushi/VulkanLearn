#pragma once

#include <GL\glew.h>

template<typename T>
class Vector3;

template <typename T>
class Matrix3x3;

template <typename T>
class Quaternion
{
public:
	Quaternion();
	Quaternion(const Quaternion<T>& q);
	Quaternion(const Matrix3x3<T>& m);
	Quaternion(T _w, T _x, T _y, T _z);
	Quaternion(const T* pData);
	Quaternion(T real, const Vector3<T>& imag);
	Quaternion(const Vector3<T>& v, T rotation);

	Matrix3x3<T> Matrix() const;

	GLboolean operator == (const Quaternion<T>& q);
	GLboolean operator != (const Quaternion<T>& q);
	Quaternion<T> operator * (const Quaternion<T>& q);

	Quaternion<T>& Normalize();
	Quaternion<T> Conjugate() const;

	Vector3<T> Rotate(const Vector3<T>& v);

	static Quaternion<T> Interpolate(const Quaternion<T>& from, const Quaternion<T>& to, T factor);

public:
	union
	{
		struct
		{
			T w, x, y, z;
		};
		struct
		{
			T real;
			Vector3<T> imag;
		};
	};
};

typedef Quaternion<GLfloat> Quaternionf;

#include "Quaternion.inl"