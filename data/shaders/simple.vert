#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inUv;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outUv;
layout (location = 2) out vec3 outViewDir;
layout (location = 3) out vec3 outWorldPos;

layout (binding = 0) uniform UBO
{
	mat4 model;
	mat4 view;
	mat4 projection;
	mat4 vulkanNDC;
	mat4 mvp;
	vec3 camPos;
}ubo;

void main() 
{
	gl_Position = ubo.mvp * vec4(inPos.xyz, 1.0);
	outNormal = inNormal;
	outUv = inUv;

	outWorldPos = (ubo.model * vec4(inPos.xyz, 1.0)).xyz;
	outViewDir = normalize(ubo.camPos - outWorldPos.xyz);
}
