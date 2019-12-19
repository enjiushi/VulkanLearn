#pragma once

template <typename T>
class Vector3;

template<typename T>
class Vector4
{
public:
	Vector4() : x(0), y(0), z(0), w(0) {}
	Vector4(T s) : x(s), y(s), z(s), w(s) {}
	Vector4(T s1, T s2, T s3, T s4) : x(s1), y(s2), z(s3), w(s4) {}
	Vector4(const Vector4<T>& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
	Vector4(const Vector3<T>& v, T s4) : x(v.x), y(v.y), z(v.z), w(s4) {}

public:
	const Vector4<T> operator + (const Vector4<T>& v) const;
	const Vector4<T> operator - (const Vector4<T>& v) const;
	T operator * (const Vector4<T>& v) const;

	const Vector4<T> operator + (T s) const;
	const Vector4<T> operator - (T s) const;
	const Vector4<T> operator * (T s) const;
	const Vector4<T> operator / (T s) const;

	Vector4<T>& operator += (T s);
	Vector4<T>& operator -= (T s);
	Vector4<T>& operator *= (T s);
	Vector4<T>& operator /= (T s);

	Vector4<T>& operator += (const Vector4<T>& v);
	Vector4<T>& operator -= (const Vector4<T>& v);
	Vector4<T>& operator *= (const Vector4<T>& v);

	Vector4<T>& operator = (T s);
	Vector4<T>& operator = (const Vector3<T>& v);

	//Vector4& operator *= (const Matrix4x4<T>& m);

	bool operator == (const Vector4<T>& v) const;
	bool operator != (const Vector4<T>& v) const;

	T& operator[] (unsigned int index);
	const T& operator[] (unsigned int index) const;

	T SquareLength() const;
	T Length() const;

	Vector4<T>& Normalize();
	const Vector4 Normal() const;
	Vector4<T>& DivHomograph();

	Vector4<float> SinglePrecision() const;
	Vector4<double> DoublePrecision() const;

	Vector3<T> xyz() const;
	Vector3<T> rgb() const { return xyz(); }
	Vector3<T> stp() const { return xyz(); }

public:
	union
	{
		T data[4];
		struct
		{
			T x, y, z, w;
		};
		struct
		{
			T r, g, b, a;
		};
		struct
		{
			T s, t, p, q;
		};
	};
};

#include "Vector4.inl"