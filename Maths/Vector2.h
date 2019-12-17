#pragma once

template<typename T>
class Vector2
{
public:
	Vector2() : x(0), y(0) {}
	Vector2(T s) : x(s), y(s) {}
	Vector2(T s1, T s2) : x(s1), y(s2) {}
	Vector2(const Vector2<T>& v) : x(v.x), y(v.y) {}

public:
	const Vector2<T> operator + (const Vector2<T>& v) const;
	const Vector2<T> operator - (const Vector2<T>& v) const;
	T operator * (const Vector2<T>& v) const;

	const Vector2<T> operator + (T s) const;
	const Vector2<T> operator - (T s) const;
	const Vector2<T> operator * (T s) const;
	const Vector2<T> operator / (T s) const;

	Vector2<T>& operator += (T s);
	Vector2<T>& operator -= (T s);
	Vector2<T>& operator *= (T s);
	Vector2<T>& operator /= (T s);

	Vector2<T>& operator += (const Vector2<T>& v);
	Vector2<T>& operator -= (const Vector2<T>& v);
	Vector2<T>& operator *= (const Vector2<T>& v);

	Vector2<T>& operator = (T s);

	bool operator == (const Vector2<T>& v) const;
	bool operator != (const Vector2<T>& v) const;

	T& operator[] (unsigned int index);
	const T& operator[] (unsigned int index) const;

	T SquareLength() const;
	T Length() const;

	Vector2<T>& Normalize();
	const Vector2<T> Normal() const;

	Vector2<T>& Negativate();
	const Vector2<T> Negative() const;

	Vector2<float> SinglePrecision() const;
	Vector2<double> DoublePrecision() const;

public:
	union
	{
		T data[2];
		struct
		{
			T x, y;
		};
		struct
		{
			T r, g;
		};
		struct
		{
			T s, t;
		};
	};
};

#include "Vector2.inl"