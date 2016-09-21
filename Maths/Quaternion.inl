#pragma once
#include "Quaternion.h"
#include "Vector3.h"

template<typename T>
Quaternion<T>::Quaternion()
{
	w = x = y = z = 0;
}

template<typename T>
Quaternion<T>::Quaternion(const Quaternion<T>& q)
{
	w = q.w;
	x = q.x;
	y = q.y;
	z = q.z;
}

template<typename T>
Quaternion<T>::Quaternion(const Matrix3x3<T>& m)
{
	T t = 1 + m.x0 + m.y1 + m.z2;

	// large enough
	if (t > static_cast<T>(0.001))
	{
		T s = sqrt(t) * static_cast<T>(2.0);
		x = (m.y2 - m.z1) / s;
		y = (m.z0 - m.x2) / s;
		z = (m.x1 - m.y0) / s;
		w = static_cast<T>(0.25) * s;
	} // else we have to check several cases
	else if (m.x0 > m.y1 && m.x0 > m.z2)
	{
		// Column 0: 
		T s = sqrt(static_cast<T>(1.0) + m.x0 - m.y1 - m.z2) * static_cast<T>(2.0);
		x = static_cast<T>(0.25) * s;
		y = (m.x1 + m.y0) / s;
		z = (m.z0 + m.x2) / s;
		w = (m.y2 - m.z1) / s;
	}
	else if (m.y1 > m.z2)
	{
		// Column 1: 
		T s = sqrt(static_cast<T>(1.0) + m.y1 - m.x0 - m.z2) * static_cast<T>(2.0);
		x = (m.x1 + m.y0) / s;
		y = static_cast<T>(0.25) * s;
		z = (m.y2 + m.z1) / s;
		w = (m.z0 - m.x2) / s;
	}
	else
	{
		// Column 2:
		T s = sqrt(static_cast<T>(1.0) + m.z2 - m.x0 - m.y1) * static_cast<T>(2.0);
		x = (m.z0 + m.x2) / s;
		y = (m.y2 + m.z1) / s;
		z = static_cast<T>(0.25) * s;
		w = (m.x1 - m.y0) / s;
	}
}

template<typename T>
Quaternion<T>::Quaternion(T _w, T _x, T _y, T _z)
{
	w = _w;
	x = _x;
	y = _y;
	z = _z;
}

template<typename T>
Quaternion<T>::Quaternion(const T* pData)
{
	w = pData[0];
	x = pData[1];
	y = pData[2];
	z = pData[3];
}

template<typename T>
Quaternion<T>::Quaternion(T real, const Vector3<T>& imag)
{
	w = real;
	imag = imag;
}

template<typename T>
Quaternion<T>::Quaternion(const Vector3<T>& v, T rotation)
{
	const T sin_a = sin(rotation / 2);
	const T cos_a = cos(rotation / 2);
	x = v.x * sin_a;
	y = v.y * sin_a;
	z = v.z * sin_a;
	w = cos_a;
}

template<typename T>
Matrix3x3<T> Quaternion<T>::Matrix() const
{
	Matrix3x3<T> resMatrix;
	resMatrix.x0 = static_cast<T>(1.0) - static_cast<T>(2.0) * (y * y + z * z);
	resMatrix.y0 = static_cast<T>(2.0) * (x * y - z * w);
	resMatrix.z0 = static_cast<T>(2.0) * (x * z + y * w);
	resMatrix.x1 = static_cast<T>(2.0) * (x * y + z * w);
	resMatrix.y1 = static_cast<T>(1.0) - static_cast<T>(2.0) * (x * x + z * z);
	resMatrix.z1 = static_cast<T>(2.0) * (y * z - x * w);
	resMatrix.x2 = static_cast<T>(2.0) * (x * z - y * w);
	resMatrix.z2 = static_cast<T>(2.0) * (y * z + x * w);
	resMatrix.y2 = static_cast<T>(1.0) - static_cast<T>(2.0) * (x * x + y * y);

	return resMatrix;
}

template<typename T>
bool Quaternion<T>::operator == (const Quaternion<T>& q)
{
	return w == q.w && x == q.x && y == q.y && z == q.z;
}

template<typename T>
bool Quaternion<T>::operator != (const Quaternion<T>& q)
{
	return !(*this == q);
}

template<typename T>
Quaternion<T> Quaternion<T>::operator * (const Quaternion<T>& q)
{
	return Quaternion<T>(w*q.w - x*q.x - y*q.y - z*q.z,
		w*q.x + x*q.w + y*q.z - z*q.y,
		w*q.y + y*q.w + z*q.x - x*q.z,
		w*q.z + z*q.w + x*q.y - y*q.x);
}

template<typename T>
Quaternion<T>& Quaternion<T>::Normalize()
{
	const T mag = sqrt(x*x + y*y + z*z + w*w);
	if (mag)
	{
		const T invMag = static_cast<T>(1.0) / mag;
		x *= invMag;
		y *= invMag;
		z *= invMag;
		w *= invMag;
	}
	return *this;
}

template<typename T>
Quaternion<T> Quaternion<T>::Conjugate() const
{
	Quaternion<T> ret = *this;
	ret.x = -ret.x;
	ret.y = -ret.y;
	ret.z = -ret.z;
	return ret;
}

template<typename T>
Vector3<T> Quaternion<T>::Rotate(const Vector3<T>& v)
{
	Quaternion<T> q2(0.f, v.x, v.y, v.z), q = *this, qinv = q;
	q.Conjugate();

	q = q.Conjugate()*q2*q;
	return Vector3<T>(q.x, q.y, q.z);
}

template<typename T>
Quaternion<T> Quaternion<T>::Interpolate(const Quaternion<T>& from, const Quaternion<T>& to, T factor)
{
	Quaternion<T> out;
	// calc cosine theta
	T cosom = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;

	// adjust signs (if necessary)
	Quaternion<T> end = to;
	if (cosom < static_cast<T>(0.0))
	{
		cosom = -cosom;
		end.x = -end.x;   // Reverse all signs
		end.y = -end.y;
		end.z = -end.z;
		end.w = -end.w;
	}

	// Calculate coefficients
	T sclp, sclq;
	// Standard case (slerp)
	T omega, sinom;
	omega = acos(cosom); // extract theta from dot product's cos theta
	sinom = sin(omega);
	sclp = sin((static_cast<T>(1.0) - factor) * omega) / sinom;
	sclq = sin(factor * omega) / sinom;

	out.x = sclp * from.x + sclq * end.x;
	out.y = sclp * from.y + sclq * end.y;
	out.z = sclp * from.z + sclq * end.z;
	out.w = sclp * from.w + sclq * end.w;

	return out;
}