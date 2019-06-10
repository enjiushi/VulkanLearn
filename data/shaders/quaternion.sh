void Conjugate(out vec4 q)
{
	q.x *= -1;
	q.y *= -1;
	q.z *= -1;
}

vec4 QuaternionMultiply(vec4 left, vec4 right)
{
	return vec4(left.w * right.xyz + right.w * left.xyz + cross(left.xyz, right.xyz), left.w * right.w - dot(left.xyz, right.xyz));
}

http://donw.io/post/dual-quaternion-skinning/
vec3 QuaternionRotate(vec4 rotation, vec3 p)
{
	return p + 2 * cross(rotation.xyz, rotation.w * p + cross(rotation, p));
}

vec4 AcquireTranslation(mat2x4 dq)
{
	vec4 revert_p = dq[0];
	Conjugate(revert_p);

	// 0.5f * t * r = q
	// r = p
	// ===> t = 2 * q * p^-1
	return 2 * QuaternionMultiply(dq[0] * revert_p);
}