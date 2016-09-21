#include "MathUtil.h"

bool FloatCompare(float a1, float a2)
{
	const static float eps = 10e-3f;
	return a1 >= a2 - eps && a1 <= a2 + eps;
}

bool FloatCompare(double a1, double a2)
{
	const static double eps = 10e-3f;
	return a1 >= a2 - eps && a1 <= a2 + eps;
}