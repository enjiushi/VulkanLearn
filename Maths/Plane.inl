#pragma once
#include "Plane.h"
#include "Vector3.h"

template<typename T>
Plane<T>::Plane(const Vector3<T>& p0, const Vector3<T>& p1, const Vector3<T>& p2, const Vector3<T>& up)
{
	Vector3<T> v0 = p2 - p0;
	Vector3<T> v1 = p2 - p1;

	v0.Normalize();
	v1.Normalize();

	normal = v0 ^ v1;

	if (normal * up < 0)
		normal *= -1.0f;

	D = normal * p2;
}

template<typename T>
Plane<T>::Plane(const Vector3<T>& normal, const Vector3<T>& p)
{
	this->normal = normal;
	D = normal * p;
}

template<typename T>
Plane<T>::Plane(const Vector3<T>& normal, T D)
{
	this->normal = normal;
	this->D = D;
}

template<typename T>
T Plane<T>::PlaneTest(const Vector3<T>& p) const
{
	return normal * p - D;
}