#version 450

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

layout (set = 0, binding = 0) uniform GlobalUniforms
{
	mat4 projection;
	mat4 vulkanNDC;
	mat4 PN;
}globalUniforms;

layout (set = 1, binding = 0) uniform PerFrameUniforms
{
	mat4 view;
	mat4 VPN;
	vec4 camPos;
}perFrameUniforms;

struct PerObjectData
{
	mat4 model;
	mat4 MVPN;
};

layout (set = 2, binding = 0) buffer PerObjectBuffer
{
	PerObjectData perObjectData[];
};

const vec3 lightPos = vec3(1000, 0, -1000);

void main() 
{
	gl_Position = perObjectData[0].MVPN * vec4(inPos.xyz, 1.0);

	outNormal = normalize(vec3(perObjectData[0].model * vec4(inNormal, 0.0)));
	outWorldPos = vec3(perObjectData[0].model * vec4(inPos, 1.0));

	outUv = inUv;
	outUv.t = 1.0 - inUv.t;

	outLightDir = vec3(lightPos - outWorldPos);
	outViewDir = vec3(perFrameUniforms.camPos.xyz - outWorldPos);

	outTangent = inTangent;
	outBitangent = normalize(cross(outNormal, normalize(vec3(perObjectData[0].model * vec4(inTangent, 0.0)))));
}
