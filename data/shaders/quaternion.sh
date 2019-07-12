void Conjugate(inout vec4 q)
{
	q.x *= -1;
	q.y *= -1;
	q.z *= -1;
}

void Conjugate(inout mat2x4 dq)
{
	Conjugate(dq[0]);
	Conjugate(dq[1]);
}

vec4 QuaternionMultiply(vec4 left, vec4 right)
{
    return vec4(left.w * right.xyz + right.w * left.xyz + cross(left.xyz, right.xyz), left.w * right.w - dot(left.xyz, right.xyz));
}

//http://donw.io/post/dual-quaternion-skinning/
vec3 QuaternionRotate(vec4 rotation, vec3 p)
{
	return p + 2 * cross(rotation.xyz, rotation.w * p + cross(rotation.xyz, p));
}

vec4 AcquireTranslation(mat2x4 dq)
{
	vec4 revert_p = dq[0];
	Conjugate(revert_p);

	// 0.5f * t * r = q
	// r = p
	// ===> t = 2 * q * p^-1
	return 2 * QuaternionMultiply(dq[1], revert_p);
}

vec3 DualQuaternionTransformPoint(mat2x4 dq, vec3 p)
{
	vec4 translate = AcquireTranslation(dq);
	vec3 ret = QuaternionRotate(dq[0], p);
	ret += translate.xyz;
	return ret;
}

vec3 DualQuaternionTransformVector(mat2x4 dq, vec3 v)
{
	vec3 ret = QuaternionRotate(dq[0], v);
	return ret;
}