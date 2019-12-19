#pragma once
#include "Vector2.h"
#include <algorithm>

template <typename T>
const Vector2<T> Vector2<T>::operator + (const Vector2<T>& v) const
{
	Vector2<T> ret_v = *this;
	ret_v += v;
	return ret_v;
}

template <typename T>
const Vector2<T> Vector2<T>::operator - (const Vector2<T>& v) const
{
	Vector2<T> ret_v = *this;
	ret_v -= v;
	return ret_v;
}

template <typename T>
T Vector2<T>::operator * (const Vector2<T>& v) const
{
	return x * v.x + y * v.y;
}

template <typename T>
const Vector2<T> Vector2<T>::operator + (T s) const
{
	Vector2<T> ret_v = *this;
	ret_v += s;
	return ret_v;
}

template <typename T>
const Vector2<T> Vector2<T>::operator - (T s) const
{
	Vector2<T> ret_v = *this;
	ret_v -= s;
	return ret_v;
}

template <typename T>
const Vector2<T> Vector2<T>::operator * (T s) const
{
	Vector2<T> ret_v = *this;
	ret_v *= s;
	return ret_v;
}

template <typename T>
const Vector2<T> Vector2<T>::operator / (T s) const
{
	Vector2<T> ret_v = *this;
	ret_v /= s;
	return ret_v;
}

template <typename T>
Vector2<T>& Vector2<T>::operator += (T s)
{
	x += s;
	y += s;
	return *this;
}

template <typename T>
Vector2<T>& Vector2<T>::operator -= (T s)
{
	x -= s;
	y -= s;
	return *this;
}

template <typename T>
Vector2<T>& Vector2<T>::operator *= (T s)
{
	x *= s;
	y *= s;
	return *this;
}

template <typename T>
Vector2<T>& Vector2<T>::operator /= (T s)
{
	x /= s;
	y /= s;
	return *this;
}

template <typename T>
Vector2<T>& Vector2<T>::operator += (const Vector2<T>& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

template <typename T>
Vector2<T>& Vector2<T>::operator -= (const Vector2<T>& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

template <typename T>
Vector2<T>& Vector2<T>::operator *= (const Vector2<T>& v)
{
	*this = *this * v;
	return *this;
}

template <typename T>
Vector2<T>& Vector2<T>::operator = (T s)
{
	x = y = s;
	return *this;
}

template <typename T>
bool Vector2<T>::operator == (const Vector2<T>& v) const
{
	return x == v.x && y == v.y;
}

template <typename T>
bool Vector2<T>::operator != (const Vector2<T>& v) const
{
	return x != v.x || y != v.y;
}

template <typename T>
T& Vector2<T>::operator[] (unsigned int index)
{
	return data[index];
}

template <typename T>
const T& Vector2<T>::operator[] (unsigned int index) const
{
	return data[index];
}

template <typename T>
T Vector2<T>::SquareLength() const
{
	return x * x + y * y;
}

template <typename T>
T Vector2<T>::Length() const
{
	return std::sqrt(SquareLength());
}

template <typename T>
Vector2<T>& Vector2<T>::Normalize()
{
	*this /= Length();
	return *this;
}

template <typename T>
const Vector2<T> Vector2<T>::Normal() const
{
	return *this / Length();
}

template <typename T>
Vector2<T>& Vector2<T>::Negativate()
{
	x = -x;
	y = -y;
	return *this;
}

template <typename T>
const Vector2<T> Vector2<T>::Negative() const
{
	return Vector2<T>() - *this;
}

template <typename T>
Vector2<float> Vector2<T>::SinglePrecision() const
{
	return { (float)x, (float)y };
}

template <typename T>
Vector2<double> Vector2<T>::DoublePrecision() const
{
	return { (double)x, (double)y };
}