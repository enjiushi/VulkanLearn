#pragma once
#include "Vector3.h"
#include <algorithm>

template <typename T>
const Vector3<T> Vector3<T>::operator + (const Vector3<T>& v) const
{
	Vector3<T> ret_v = *this;
	ret_v += v;
	return ret_v;
}

template <typename T>
const Vector3<T> Vector3<T>::operator - (const Vector3<T>& v) const
{
	Vector3<T> ret_v = *this;
	ret_v -= v;
	return ret_v;
}

template <typename T>
T Vector3<T>::operator * (const Vector3<T>& v) const
{
	return x * v.x + y * v.y + z * v.z;
}

template <typename T>
const Vector3<T> Vector3<T>::operator ^ (const Vector3<T>& v) const
{
	return Vector3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

template <typename T>
const Vector3<T> Vector3<T>::operator + (T s) const
{
	Vector3<T> ret_v = *this;
	ret_v += s;
	return ret_v;
}

template <typename T>
const Vector3<T> Vector3<T>::operator - (T s) const
{
	Vector3<T> ret_v = *this;
	ret_v -= s;
	return ret_v;
}

template <typename T>
const Vector3<T> Vector3<T>::operator * (T s) const
{
	Vector3<T> ret_v = *this;
	ret_v *= s;
	return ret_v;
}

template <typename T>
const Vector3<T> Vector3<T>::operator / (T s) const
{
	Vector3<T> ret_v = *this;
	ret_v /= s;
	return ret_v;
}

template <typename T>
Vector3<T>& Vector3<T>::operator += (T s)
{
	x += s;
	y += s;
	z += s;
	return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator -= (T s)
{
	x -= s;
	y -= s;
	z -= s;
	return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator *= (T s)
{
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator /= (T s)
{
	x /= s;
	y /= s;
	z /= s;
	return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator += (const Vector3<T>& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator -= (const Vector3<T>& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator *= (const Vector3<T>& v)
{
	*this = (*this * v);
	return *this;
}

template <typename T>
bool Vector3<T>::operator == (const Vector3<T>& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

template <typename T>
bool Vector3<T>::operator != (const Vector3<T>& v) const
{
	return x != v.x || y != v.y || z != v.z;
}

template <typename T>
T& Vector3<T>::operator[] (unsigned int index)
{
	return data[index];
}

template <typename T>
const T& Vector3<T>::operator[] (unsigned int index) const
{
	return data[index];
}

template <typename T>
T Vector3<T>::SquareLength() const
{
	return x * x + y * y + z * z;
}

template <typename T>
T Vector3<T>::Length() const
{
	return std::sqrt(SquareLength());
}

template <typename T>
Vector3<T>& Vector3<T>::Normalize()
{
	*this /= Length();
	return *this;
}

template <typename T>
const Vector3<T> Vector3<T>::Normalize() const
{
	return *this / Length();
}

template <typename T>
Vector3<T>& Vector3<T>::Negative()
{
	x = -x;
	y = -y;
	z = -z;
	return *this;
}

template <typename T>
const Vector3<T> Vector3<T>::Negative() const
{
	return Vector3<T>() - *this;
}