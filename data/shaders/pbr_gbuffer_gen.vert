#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 3) in vec2 inUv;
layout (location = 4) in vec3 inTangent;

layout (location = 0) out vec2 outUv;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outTangent;
layout (location = 3) out vec3 outBitangent;
layout (location = 4) flat out int perMaterialIndex;
layout (location = 5) flat out int perObjectIndex;
layout (location = 6) out vec3 outWorldPos;

#include "uniform_layout.h"

void main() 
{
	perObjectIndex = objectDataIndex[gl_DrawID].perObjectIndex;

	gl_Position = perObjectData[perObjectIndex].MVPN * vec4(inPos.xyz, 1.0);

	outNormal = normalize(vec3(perObjectData[perObjectIndex].model * vec4(inNormal, 0.0)));

	outUv = inUv;
	outUv.t = 1.0 - inUv.t;

	outTangent = normalize(vec3(perObjectData[perObjectIndex].model * vec4(inTangent, 0.0)));
	outBitangent = normalize(cross(outNormal, outTangent));

	perMaterialIndex = objectDataIndex[gl_DrawID].perMaterialIndex;
}
