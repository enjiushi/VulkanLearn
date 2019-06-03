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
	DualQuaternion(T _w, T _x, T _y, T _z, T _dw, T _dx, T _dy, T _dz);
	DualQuaternion(const T* pData);

	bool operator == (const DualQuaternion<T>& dq) const;
	bool operator != (const DualQuaternion<T>& dq) const;
	DualQuaternion<T>& operator *= (const DualQuaternion<T>& dq);

	Quaternion<T> AcquireRotation() const;
	Vector3<T> AcquireTranslation() const;

	DualQuaternion<T> operator * (const DualQuaternion<T>& dq);

	DualQuaternion<T>& Normalize();
	DualQuaternion<T> GetConjugate() const;
	DualQuaternion<T>& Conjugate();

	Vector3<T> Transform(const Vector3<T>& input);

	static DualQuaternion<T> DLB(const DualQuaternion<T>& from, const DualQuaternion<T>& to, float factor);

public:
	union
	{
		struct
		{
			T w, x, y, z;
			T dw, dx, dy, dz;
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