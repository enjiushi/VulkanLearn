#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 3) in vec2 inUv;
layout (location = 0) out vec2 outUv;

layout (set = 0, binding = 0) uniform GlobalUniforms
{
	mat4 projection;
	mat4 vulkanNDC;
	mat4 PN;
}globalUniforms;

void main() 
{
	//gl_Position = globalUniforms.vulkanNDC * vec4(inPos.xyz, 1.0);
	//gl_Position.z = 0;
	//outUv = inUv.st;

	outUv = vec2(gl_VertexIndex & 1, gl_VertexIndex >> 1);
	gl_Position = vec4(outUv * 2.0f - 1.0f, 0.0f, 1.0f);
	gl_Position.y *= -1.0;
}
