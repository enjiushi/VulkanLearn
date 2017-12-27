#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 3) in vec2 inUv;
layout (location = 4) in vec3 inTangent;

layout (location = 0) out vec2 outUv;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outWorldPos;
layout (location = 3) out vec3 outLightDir;
layout (location = 4) out vec3 outViewDir;
layout (location = 5) out vec3 outTangent;
layout (location = 6) out vec3 outBitangent;
layout (location = 7) flat out int perMaterialIndex;

#include "uniform_layout.h"

const vec3 lightPos = vec3(1000, 0, -1000);

void main() 
{
	int perObjectIndex = objectDataIndex[gl_DrawID].perObjectIndex;

	gl_Position = perObjectData[perObjectIndex].MVPN * vec4(inPos.xyz, 1.0);

	outNormal = normalize(vec3(perObjectData[perObjectIndex].model * vec4(inNormal, 0.0)));
	outWorldPos = vec3(perObjectData[perObjectIndex].model * vec4(inPos, 1.0));

	outUv = inUv;
	outUv.t = 1.0 - inUv.t;

	outLightDir = vec3(lightPos - outWorldPos);
	outViewDir = vec3(perFrameData.camPos.xyz - outWorldPos);

	outTangent = inTangent;
	outBitangent = normalize(cross(outNormal, normalize(vec3(perObjectData[perObjectIndex].model * vec4(inTangent, 0.0)))));

	perMaterialIndex = objectDataIndex[gl_DrawID].perMaterialIndex;
}
