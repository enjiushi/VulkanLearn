#pragma once
#include "Vector4.h"
#include <algorithm>
#include "Vector3.inl"

template <typename T>
const Vector4<T> Vector4<T>::operator + (const Vector4<T>& v) const
{
	Vector4<T> ret_v = *this;
	ret_v += v;
	return ret_v;
}

template <typename T>
const Vector4<T> Vector4<T>::operator - (const Vector4<T>& v) const
{
	Vector4<T> ret_v = *this;
	ret_v -= v;
	return ret_v;
}

template <typename T>
T Vector4<T>::operator * (const Vector4<T>& v) const
{
	return x * v.x + y * v.y + z * v.z + w * v.w;
}

template <typename T>
const Vector4<T> Vector4<T>::operator + (T s) const
{
	Vector4<T> ret_v = *this;
	ret_v += s;
	return ret_v;
}

template <typename T>
const Vector4<T> Vector4<T>::operator - (T s) const
{
	Vector4<T> ret_v = *this;
	ret_v -= s;
	return ret_v;
}

template <typename T>
const Vector4<T> Vector4<T>::operator * (T s) const
{
	Vector4<T> ret_v = *this;
	ret_v *= s;
	return ret_v;
}

template <typename T>
const Vector4<T> Vector4<T>::operator / (T s) const
{
	Vector4<T> ret_v = *this;
	ret_v /= s;
	return ret_v;
}

template <typename T>
Vector4<T>& Vector4<T>::operator += (T s)
{
	x += s;
	y += s;
	z += s;
	w += s;
	return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator -= (T s)
{
	x -= s;
	y -= s;
	z -= s;
	w -= s;
	return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator *= (T s)
{
	x *= s;
	y *= s;
	z *= s;
	w *= s;
	return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator /= (T s)
{
	x /= s;
	y /= s;
	z /= s;
	w /= s;
	return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator += (const Vector4<T>& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator -= (const Vector4<T>& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator *= (const Vector4<T>& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
	return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator = (T s)
{
	x = y = z = w = s;
	return *this;
}

template <typename T>
Vector4<T>& Vector4<T>::operator = (const Vector3<T>& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

template <typename T>
bool Vector4<T>::operator == (const Vector4<T>& v) const
{
	return x == v.x && y == v.y && z == v.z && w == v.w;
}

template <typename T>
bool Vector4<T>::operator != (const Vector4<T>& v) const
{
	return x != v.x || y != v.y || z != v.z || w != v.w;
}

template <typename T>
T& Vector4<T>::operator[] (unsigned int index)
{
	return data[index];
}

template <typename T>
const T& Vector4<T>::operator[] (unsigned int index) const
{
	return data[index];
}

template <typename T>
T Vector4<T>::SquareLength() const
{
	return x * x + y * y + z * z + w * w;
}

template <typename T>
T Vector4<T>::Length() const
{
	return std::sqrt(SquareLength());
}

template <typename T>
Vector4<T>& Vector4<T>::Normalize()
{
	*this /= Length();
	return *this;
}

template <typename T>
const Vector4<T> Vector4<T>::Normal() const
{
	return *this / Length();
}

template <typename T>
Vector4<T>& Vector4<T>::DivHomograph()
{
	*this /= w;
	return *this;
}

template <typename T>
Vector4<float> Vector4<T>::SinglePrecision() const
{
	return { (float)x, (float)y, (float)z, (float)w };
}

template <typename T>
Vector4<double> Vector4<T>::DoublePrecision() const
{
	return { (double)x, (double)y, (double)z, (double)w };
}


template <typename T>
Vector3<T> Vector4<T>::xyz() const
{
	return Vector3<T>(data[0], data[1], data[2]);
}