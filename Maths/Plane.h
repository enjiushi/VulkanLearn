#pragma once

template <typename T>
class Vector3;

template<typename T>
class Plane
{
public:
	Plane() = default;

	// 3 points one the plane, and an up vector
	Plane(const Vector3<T>& p0, const Vector3<T>& p1, const Vector3<T>& p2, const Vector3<T>& up);

	// normal and a point one the plane
	Plane(const Vector3<T>& normal, const Vector3<T>& p);

	// strictly follows the plane equation
	Plane(const Vector3<T>& normal, T D);

public:
	// Test which side of the plane the input "p" locates(positive lies on positive normal side, and vice versa)
	T PlaneTest(const Vector3<T>& p) const;

public:
	// normal * x = D
	Vector3<T>	normal;
	float		D = 0;
};

#include "Plane.inl"

typedef Plane<float>	Planef;
typedef Plane<double>	Planed;