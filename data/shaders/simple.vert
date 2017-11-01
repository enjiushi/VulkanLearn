#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 3) in vec2 inUv;
layout (location = 0) out vec2 outUv;

struct PerObjectData
{
	mat4 model;
	mat4 MVPN;
};

layout (set = 2, binding = 0) buffer PerObjectBuffer
{
	PerObjectData perObjectData[];
};

void main() 
{
	gl_Position = perObjectData[0].MVPN * vec4(inPos.xyz, 1.0);

	outUv = inUv.st;
	outUv.t = 1.0 - outUv.t;
}
