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
	const Vector3<T> operator + (const Vector3<T>& v) const;
	const Vector3<T> operator - (const Vector3<T>& v) const;
	T operator * (const Vector3<T>& v) const;
	const Vector3<T> operator ^ (const Vector3<T>& v) const;

	const Vector3<T> operator + (T s) const;
	const Vector3<T> operator - (T s) const;
	const Vector3<T> operator * (T s) const;
	const Vector3<T> operator / (T s) const;

	Vector3<T>& operator += (T s);
	Vector3<T>& operator -= (T s);
	Vector3<T>& operator *= (T s);
	Vector3<T>& operator /= (T s);

	Vector3<T>& operator += (const Vector3<T>& v);
	Vector3<T>& operator -= (const Vector3<T>& v);
	Vector3<T>& operator *= (const Vector3<T>& v);

	bool operator == (const Vector3<T>& v) const;
	bool operator != (const Vector3<T>& v) const;

	T& operator[] (unsigned int index);
	const T& operator[] (unsigned int index) const;

	T SquareLength() const;
	T Length() const;

	Vector3<T>& Normalize();
	const Vector3<T> Normal() const;

	Vector3<T>& Negativate();
	const Vector3<T> Negative() const;

	Vector3<T> Orthogonal() const;

	Vector3<float> SinglePrecision() const;
	Vector3<double> DoublePrecision() const;

	static const Vector3<T> Forward() { return Vector3<T>(0.0, 0.0, 1.0); }
	static const Vector3<T> Upward() { return Vector3<T>(0.0, 1.0, 0.0); }
	static const Vector3<T> Left() { return Vector3<T>(1.0, 0.0, 0.0); }

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