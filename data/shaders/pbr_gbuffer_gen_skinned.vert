#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 3) in vec2 inUv;
layout (location = 4) in vec3 inTangent;
layout (location = 5) in vec4 inBoneWeight;
layout (location = 6) in uint inBoneIndices;

layout (location = 0) out vec2 outUv;
layout (location = 1) out vec3 outCSNormal;
layout (location = 2) out vec3 outCSTangent;
layout (location = 3) out vec3 outCSBitangent;
layout (location = 4) flat out int perMaterialIndex;
layout (location = 5) flat out int perObjectIndex;
layout (location = 6) out vec3 outCSPosition;
layout (location = 7) noperspective out vec2 outScreenPosition;
layout (location = 8) out vec3 outPrevCSPosition;

#include "uniform_layout.sh"
#include "quaternion.sh"
#include "utilities.sh"

void main() 
{
	int indirectIndex = GetIndirectIndex(gl_DrawID, gl_InstanceIndex);

	perObjectIndex = objectDataIndex[indirectIndex].perObjectIndex;

	int perAnimationChunkIndex = objectDataIndex[indirectIndex].utilityIndex;

	vec4 bone_weights = inBoneWeight;
	uvec4 boneIndices = uvec4(perFrameBoneChunkIndirect[animationData[perAnimationChunkIndex].boneChunkIndexOffset + (inBoneIndices >> 0) & 255],
								perFrameBoneChunkIndirect[animationData[perAnimationChunkIndex].boneChunkIndexOffset + (inBoneIndices >> 8) & 255],
								perFrameBoneChunkIndirect[animationData[perAnimationChunkIndex].boneChunkIndexOffset + (inBoneIndices >> 16) & 255],
								perFrameBoneChunkIndirect[animationData[perAnimationChunkIndex].boneChunkIndexOffset + (inBoneIndices >> 24) & 255]);

	mat2x4 dq0 = perFrameBoneData[boneIndices.x].currAnimationDQ;
    mat2x4 dq1 = perFrameBoneData[boneIndices.y].currAnimationDQ;
    mat2x4 dq2 = perFrameBoneData[boneIndices.z].currAnimationDQ;
    mat2x4 dq3 = perFrameBoneData[boneIndices.w].currAnimationDQ;

    // Ensure all bone transforms are in the same neighbourhood
    if (dot(dq0[0], dq1[0]) < 0.0) bone_weights.y *= -1.0;
    if (dot(dq0[0], dq2[0]) < 0.0) bone_weights.z *= -1.0;
    if (dot(dq0[0], dq3[0]) < 0.0) bone_weights.w *= -1.0;

    // Blend
    mat2x4 currDQ =
        bone_weights.x * dq0 +
        bone_weights.y * dq1 +
        bone_weights.z * dq2 +
        bone_weights.w * dq3;

	// NOTE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Since minus dual quaternion equals original one:
	// DQ = p + sigma * q, p = r, q = 0.5 * t * r
	// r = cos(theta) + nsin(theta)
	// r = -r
	// =========>-r + sigma * 0.5 * t * (-r) = -r - sigma * 0.5 * t * r = -p - sigma * q = -DQ
	// This proves minus DQ equals transform of original DQ
	// However, linear combination will be changed, e.g:
	// DQ0 * 0.3 + DQ1 * 0.7 ===> DQ0 * 0.3 + DQ1 * (-0.7)
	// Though DQ1 and -DQ1 represent same transform, it still breaks linear combination that the result is no longer a normalized dual quaternion
	// SO, WE HAVE TO RE NORMALIZE INTERPOLATED RESULT, WE HAVE TO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	float len = length(currDQ[0]);
	currDQ /= len;

	bone_weights = inBoneWeight;

	dq0 = perFrameBoneData[boneIndices.x].prevAnimationDQ;
    dq1 = perFrameBoneData[boneIndices.y].prevAnimationDQ;
    dq2 = perFrameBoneData[boneIndices.z].prevAnimationDQ;
    dq3 = perFrameBoneData[boneIndices.w].prevAnimationDQ;

    // Ensure all bone transforms are in the same neighbourhood
    if (dot(dq0[0], dq1[0]) < 0.0) bone_weights.y *= -1.0;
    if (dot(dq0[0], dq2[0]) < 0.0) bone_weights.z *= -1.0;
    if (dot(dq0[0], dq3[0]) < 0.0) bone_weights.w *= -1.0;

    // Blend
    mat2x4 prevDQ =
        bone_weights.x * dq0 +
        bone_weights.y * dq1 +
        bone_weights.z * dq2 +
        bone_weights.w * dq3;

	len = length(prevDQ[0]);
	prevDQ /= len;

	vec3 animated_pos = DualQuaternionTransformPoint(currDQ, inPos);
	vec3 prev_animated_pos = DualQuaternionTransformPoint(prevDQ, inPos);
	vec3 animated_normal = DualQuaternionTransformVector(currDQ, inNormal);
	vec3 animated_tangent = DualQuaternionTransformVector(currDQ, inTangent);

	gl_Position = perObjectData[perObjectIndex].MVP * vec4(animated_pos.xyz, 1.0);

	outCSNormal = normalize(vec3(perObjectData[perObjectIndex].MV * vec4(animated_normal, 0.0)));
	outCSPosition = (perObjectData[perObjectIndex].MV * vec4(animated_pos.xyz, 1.0)).xyz;
	outPrevCSPosition = (perObjectData[perObjectIndex].prevMV * vec4(prev_animated_pos.xyz, 1.0)).xyz;
	outScreenPosition = gl_Position.xy / gl_Position.w;

	outUv = inUv;
	outUv.t = 1.0 - inUv.t;

	outCSTangent = normalize(vec3(perObjectData[perObjectIndex].MV * vec4(animated_tangent, 0.0)));
	outCSBitangent = normalize(cross(outCSNormal, outCSTangent));

	perMaterialIndex = objectDataIndex[indirectIndex].perMaterialIndex;
}
