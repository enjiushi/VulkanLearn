#pragma once
#include "PyramidFrustum.h"
#include "Vector3.h"
#include "Plane.h"
#include "Quaternion.h"

template <typename T>
PyramidFrustum<T>::PyramidFrustum(const Vector3<T>& head, const Vector3<T>& bottomLeft, const Vector3<T>& bottomRight, const Vector3<T>& topLeft, const Vector3<T>& topRight)
{
	planes[FrustumFace_LEFT]	= { bottomLeft,		topLeft,	 head, Vector3<T>(bottomRight - bottomLeft).Normal() };
	planes[FrustumFace_RIGHT]	= { bottomRight,	topRight,	 head, Vector3<T>(bottomLeft - bottomRight).Normal() };
	planes[FrustumFace_BOTTOM]	= { bottomLeft,		bottomRight, head, Vector3<T>(topLeft - bottomLeft).Normal() };
	planes[FrustumFace_TOP]		= { topLeft,		topRight,	 head, Vector3<T>(bottomLeft - topLeft).Normal() };
}

template <typename T>
PyramidFrustum<T>::PyramidFrustum(const Vector3<T>& head, const Vector3<T>& lookAt, float fovv, float aspect)
{
	float tangentFOV_2_v = std::tanf(fovv);
	float tangentFOV_2_h = aspect * tangentFOV_2_v;

	Quaternion<T> q({ 0, 0, -1 }, lookAt);

	// Bottom left, bottom right, upper left, upper right corners of the view frustum based one the unit length 1
	Vector3<T> bottomLeft = q.Rotate({ -tangentFOV_2_h, -tangentFOV_2_v, -1.0f });
	Vector3<T> bottomRight = q.Rotate({ tangentFOV_2_h, -tangentFOV_2_v, -1.0f });
	Vector3<T> topLeft = q.Rotate({ -tangentFOV_2_h, tangentFOV_2_v, -1.0f });
	Vector3<T> topRight = q.Rotate({ tangentFOV_2_h, tangentFOV_2_v, -1.0f });

	planes[FrustumFace_LEFT]	= { bottomLeft,		topLeft,	 head, Vector3<T>(bottomRight - bottomLeft).Normal() };
	planes[FrustumFace_RIGHT]	= { bottomRight,	topRight,	 head, Vector3<T>(bottomLeft - bottomRight).Normal() };
	planes[FrustumFace_BOTTOM]	= { bottomLeft,		bottomRight, head, Vector3<T>(topLeft - bottomLeft).Normal() };
	planes[FrustumFace_TOP]		= { topLeft,		topRight,	 head, Vector3<T>(bottomLeft - topLeft).Normal() };
}

template <typename T>
bool PyramidFrustum<T>::Contain(const Vector3<T>& p) const
{
	for (uint32_t i = 0; i < FrustumFace_COUNT; i++)
		if (planes[i].PlaneTest(p) < 0)
			return false;

	return true;
}