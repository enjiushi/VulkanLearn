#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 color = pow(vec3(0.5), vec3(1.0 / 2.2));
	outFragColor = vec4(color, 1.0);
}