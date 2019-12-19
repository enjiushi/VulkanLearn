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
const Vector3<T> Vector3<T>::Normal() const
{
	return *this / Length();
}

template <typename T>
Vector3<T>& Vector3<T>::Negativate()
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

template <typename T>
Vector3<T> Vector3<T>::Orthogonal() const
{
	T _x = abs(x);
	T _y = abs(y);
	T _z = abs(z);

	Vector3<T> other = _x < _y ? (_x < _z ? Vector3<T>(1, 0, 0) : Vector3<T>(0, 0, 1)) : (_y < _z ? Vector3<T>(0, 1, 0) : Vector3<T>(0, 0, 1));
	return *this ^ other;
}

template <typename T>
Vector3<float> Vector3<T>::SinglePrecision() const
{
	return { (float)x, (float)y, (float)z };
}

template <typename T>
Vector3<double> Vector3<T>::DoublePrecision() const
{
	return { (double)x, (double)y, (double)z };
}