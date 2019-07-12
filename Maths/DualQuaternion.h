#pragma once

template<typename T>
class Vector3;

template <typename T>
class Matrix3x3;

template <typename T>
class Matrix4x4;

template <typename T>
class Quaternion;

template <typename T>
class DualQuaternion
{
public:
	DualQuaternion();
	DualQuaternion(const DualQuaternion<T>& dq);
	DualQuaternion(const Quaternion<T>& r, const Quaternion<T>& t);
	DualQuaternion(const Quaternion<T>& r, const Vector3<T>& t);
	DualQuaternion(T _x, T _y, T _z, T _w, T _dx, T _dy, T _dz, T _dw);
	DualQuaternion(const T* pData);

	bool operator == (const DualQuaternion<T>& dq) const;
	bool operator != (const DualQuaternion<T>& dq) const;

	DualQuaternion<T>& operator *= (const DualQuaternion<T>& dq);
	DualQuaternion<T>& operator += (const DualQuaternion<T>& dq);
	DualQuaternion<T>& operator -= (const DualQuaternion<T>& dq);

	DualQuaternion<T>& operator *= (T s);
	DualQuaternion<T>& operator += (T s);
	DualQuaternion<T>& operator -= (T s);

	const DualQuaternion<T> operator * (const DualQuaternion<T>& dq) const;
	const DualQuaternion<T> operator + (const DualQuaternion<T>& dq) const;
	const DualQuaternion<T> operator - (const DualQuaternion<T>& dq) const;

	const DualQuaternion<T> operator * (T s) const;
	const DualQuaternion<T> operator + (T s) const;
	const DualQuaternion<T> operator - (T s) const;

	Quaternion<T> AcquireRotation() const;
	Vector3<T> AcquireTranslation() const;

	DualQuaternion<T>& Normalize();
	const DualQuaternion<T> GetConjugate() const;
	DualQuaternion<T>& Conjugate();

	Vector3<T> Transform(const Vector3<T>& input);

	static DualQuaternion<T> DLB(const DualQuaternion<T>& from, const DualQuaternion<T>& to, float factor);

public:
	union
	{
		struct
		{
			T x, y, z, w;
			T dx, dy, dz, dw;
		};
		struct
		{
			Quaternion<T> real;
			Quaternion<T> dual;
		};
	};
};

typedef DualQuaternion<float> DualQuaternionf;

#include "DualQuaternion.inl"