#pragma once
#include "Quaternion.h"
#include "Vector3.h"

template<typename T>
DualQuaternion<T>::DualQuaternion()
{
	real = Quaternionf(0, 0, 0, 1);
	dual = Quaternionf(0, 0, 0, 0);
}

template<typename T>
DualQuaternion<T>::DualQuaternion(const DualQuaternion<T>& dq)
{
	*this = dq;
}

template<typename T>
DualQuaternion<T>::DualQuaternion(const Quaternion<T>& r, const Quaternion<T>& t)
{
	real = r;
	dual = t;
}

// Rotate first, and then translate
template<typename T>
DualQuaternion<T>::DualQuaternion(const Quaternion<T>& r, const Vector3<T>& t)
{
	real = r;
	dual = Quaternion<T>(0, t) * r * 0.5f;
}

template<typename T>
DualQuaternion<T>::DualQuaternion(T _x, T _y, T _z, T _w, T _dx, T _dy, T _dz, T _dw)
{
	real = Quaternion<T>(_x, _y, _z, _w);
	dual = Quaternion<T>(_dx, _dy, _dz, _dw);
}

template<typename T>
DualQuaternion<T>::DualQuaternion(const T* pData)
{
	real = Quaternion<T>(pData[0], pData[1], pData[2], pData[3]);
	dual = Quaternion<T>(pData[4], pData[5], pData[6], pData[7]);
}

template<typename T>
bool DualQuaternion<T>::operator == (const DualQuaternion<T>& dq) const
{
	return (real == dq.real) && (dual == dq.dual);
}

template<typename T>
bool DualQuaternion<T>::operator != (const DualQuaternion<T>& dq) const
{
	return *this != dq;
}

template<typename T>
Quaternion<T> DualQuaternion<T>::AcquireRotation() const
{
	return real;
}

template<typename T>
Vector3<T> DualQuaternion<T>::AcquireTranslation() const
{
	// when it comes to acquire translation
	// always assume the order is translate after rotation
	// i.e, 1/2 * t * r = dual ====> t = 2 * dual * real^-1
	Quaternion<T> t = dual * real.GetConjugate() * static_cast<T>(2.0);
	return {t.x, t.y, t.z};
}

template<typename T>
DualQuaternion<T>& DualQuaternion<T>::operator *= (const DualQuaternion<T>& dq)
{
	real = real * dq.real;
	dual = real * dq.dual + dual * dq.real;
	return *this;
}

template<typename T>
DualQuaternion<T>& DualQuaternion<T>::operator += (const DualQuaternion<T>& dq)
{
	real += dq.real;
	dual += dq.dual;
	return *this;
}

template<typename T>
DualQuaternion<T>& DualQuaternion<T>::operator -= (const DualQuaternion<T>& dq)
{
	real -= dq.real;
	dual -= dq.dual;
	return *this;
}

template<typename T>
DualQuaternion<T>& DualQuaternion<T>::operator *= (T s)
{
	real *= s;
	dual *= s;
	return *this;
}

template<typename T>
DualQuaternion<T>& DualQuaternion<T>::operator += (T s)
{
	real += s;
	dual += s;
	return *this;
}

template<typename T>
DualQuaternion<T>& DualQuaternion<T>::operator -= (T s)
{
	real -= s;
	dual -= s;
	return *this;
}


template<typename T>
const DualQuaternion<T> DualQuaternion<T>::operator * (const DualQuaternion<T>& dq) const
{
	DualQuaternion<T> ret = *this;
	ret *= dq;
	return ret;
}

template<typename T>
const DualQuaternion<T> DualQuaternion<T>::operator + (const DualQuaternion<T>& dq) const
{
	DualQuaternion<T> ret = *this;
	ret += dq;
	return ret;
}

template<typename T>
const DualQuaternion<T> DualQuaternion<T>::operator - (const DualQuaternion<T>& dq) const
{
	DualQuaternion<T> ret = *this;
	ret -= dq;
	return ret;
}

template<typename T>
const DualQuaternion<T> DualQuaternion<T>::operator * (T s) const
{
	DualQuaternion<T> ret = *this;
	ret *= s;
	return ret;
}

template<typename T>
const DualQuaternion<T> DualQuaternion<T>::operator + (T s) const
{
	DualQuaternion<T> ret = *this;
	ret += s;
	return ret;
}

template<typename T>
const DualQuaternion<T> DualQuaternion<T>::operator - (T s) const
{
	DualQuaternion<T> ret = *this;
	ret -= s;
	return ret;
}

template<typename T>
DualQuaternion<T>& DualQuaternion<T>::Normalize()
{
	T mag = real.Mag();
	T invMag = static_cast<T>(1.0) / mag;
	real = real * invMag;
	dual = dual * invMag;

	return *this;
}

template<typename T>
const DualQuaternion<T> DualQuaternion<T>::GetConjugate() const
{
	return DualQuaternion<T>(real.GetConjugate(), dual.GetConjugate());
}

template<typename T>
DualQuaternion<T>& DualQuaternion<T>::Conjugate()
{
	real.Conjugate();
	dual.Conjugate();

	return *this;
}

template<typename T>
Vector3<T> DualQuaternion<T>::Transform(const Vector3<T>& input)
{
	Quaternion<T> r = AcquireRotation();
	Vector3<T> t = AcquireTranslation();

	return r.Rotate(input) + t;
}

template<typename T>
DualQuaternion<T> DualQuaternion<T>::DLB(const DualQuaternion<T>& from, const DualQuaternion<T>& to, float factor)
{
	DualQuaternion<T> ret;
	
	ret.real.x = from.real.x * factor + to.real.x * (static_cast<T>(1.0) - factor);
	ret.real.y = from.real.y * factor + to.real.y * (static_cast<T>(1.0) - factor);
	ret.real.z = from.real.z * factor + to.real.z * (static_cast<T>(1.0) - factor);
	ret.real.w = from.real.w * factor + to.real.w * (static_cast<T>(1.0) - factor);

	ret.dual.x = from.dual.x * factor + to.dual.x * (static_cast<T>(1.0) - factor);
	ret.dual.y = from.dual.y * factor + to.dual.y * (static_cast<T>(1.0) - factor);
	ret.dual.z = from.dual.z * factor + to.dual.z * (static_cast<T>(1.0) - factor);
	ret.dual.w = from.dual.w * factor + to.dual.w * (static_cast<T>(1.0) - factor);

	ret.Normalize();

	return ret;
}