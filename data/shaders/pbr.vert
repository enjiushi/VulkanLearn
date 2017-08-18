#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inUv;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

layout (location = 0) out vec3 outUv;
layout (location = 1) out vec3 outViewDir;
layout (location = 2) out vec3 outLightDir;
layout (location = 3) out vec3 outNormal;
layout (location = 4) out vec3 outTangent;
layout (location = 5) out vec3 outBiTangent;

layout (binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 vulkanNDC;
	mat4 mvp;
	vec3 camPos;
	float roughness;
}ubo;

const vec3 lightPos = vec3(500, 0, 500);

void main() 
{
	gl_Position = ubo.mvp * vec4(inPos.xyz, 1.0);

	outTangent = normalize(vec3(ubo.model * vec4(inTangent, 0.0)));
	outBiTangent = normalize(vec3(ubo.model * vec4(inBitangent, 0.0)));
	outNormal = normalize(vec3(ubo.model * vec4(inNormal, 0.0)));

	//mat3 TBN = transpose(mat3(T, B, N));

	outUv = inUv;
	outUv.t = 1.0 - inUv.t;

	vec3 worldPos = (ubo.model * vec4(inPos.xyz, 1.0)).xyz;
	outViewDir = normalize(ubo.camPos - worldPos);
	outLightDir = normalize(lightPos - worldPos);
}
