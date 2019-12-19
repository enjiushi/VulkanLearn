#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 3) in vec2 inUv;
layout (location = 4) in vec3 inTangent;

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

void main() 
{
	int indirectIndex = GetIndirectIndex(gl_DrawID, gl_InstanceIndex);

	perObjectIndex = objectDataIndex[indirectIndex].perObjectIndex;

	gl_Position = perObjectData[perObjectIndex].MVP * vec4(inPos.xyz, 1.0);

	outCSNormal = normalize(vec3(perObjectData[perObjectIndex].MV * vec4(inNormal, 0.0)));
	outCSPosition = (perObjectData[perObjectIndex].MV * vec4(inPos, 1.0)).xyz;
	outPrevCSPosition = (perObjectData[perObjectIndex].prevMV * vec4(inPos.xyz, 1.0)).xyz;
	outScreenPosition = gl_Position.xy / gl_Position.w;

	outUv = inUv;
	outUv.t = 1.0 - inUv.t;

	outCSTangent = normalize(vec3(perObjectData[perObjectIndex].MV * vec4(inTangent, 0.0)));
	outCSBitangent = normalize(cross(outCSNormal, outCSTangent));

	perMaterialIndex = objectDataIndex[indirectIndex].perMaterialIndex;
}
