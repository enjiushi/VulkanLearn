#pragma once

template<typename T>
class Vector3
{
public:
	Vector3() : x(0), y(0), z(0) {}
	Vector3(T s) : x(s), y(s), z(s) {}
	Vector3(T s1, T s2, T s3) : x(s1), y(s2), z(s3) {}
	Vector3(const Vector3<T>& v) : x(v.x), y(v.y), z(v.z) {}

public:
	const Vector3 operator + (const Vector3<T>& v) const;
	const Vector3 operator - (const Vector3<T>& v) const;
	T operator * (const Vector3<T>& v) const;
	const Vector3 operator ^ (const Vector3<T>& v) const;

	const Vector3 operator + (T s) const;
	const Vector3 operator - (T s) const;
	const Vector3 operator * (T s) const;
	const Vector3 operator / (T s) const;

	Vector3& operator += (T s);
	Vector3& operator -= (T s);
	Vector3& operator *= (T s);
	Vector3& operator /= (T s);

	Vector3& operator += (const Vector3<T>& v);
	Vector3& operator -= (const Vector3<T>& v);
	Vector3& operator *= (const Vector3<T>& v);

	//Vector3& operator *= (const Matrix4x4<T>& m);

	bool operator == (const Vector3<T>& v) const;
	bool operator != (const Vector3<T>& v) const;

	T& operator[] (unsigned int index);
	const T& operator[] (unsigned int index) const;

	T SquareLength() const;
	T Length() const;

	Vector3& Normalize();
	const Vector3 Normal() const;

	Vector3& Negativate();
	const Vector3 Negative() const;

	Vector3 Orthogonal() const;

	static const Vector3 Forward() { return Vector3<T>(0.0, 0.0, 1.0); }
	static const Vector3 Upward() { return Vector3<T>(0.0, 1.0, 0.0); }
	static const Vector3 Left() { return Vector3<T>(1.0, 0.0, 0.0); }

public:
	union
	{
		T data[3];
		struct
		{
			T x, y, z;
		};
		struct
		{
			T r, g, b;
		};
		struct
		{
			T s, t, p;
		};
	};
};

#include "Vector3.inl"