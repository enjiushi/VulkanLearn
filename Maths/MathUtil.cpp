#include "stdafx.h"
#include "MathUtil.h"

GLboolean FloatCompare(GLfloat a1, GLfloat a2)
{
	const static GLfloat eps = 10e-3f;
	return a1 >= a2 - eps && a1 <= a2 + eps;
}

GLboolean FloatCompare(GLdouble a1, GLdouble a2)
{
	const static GLdouble eps = 10e-3f;
	return a1 >= a2 - eps && a1 <= a2 + eps;
}