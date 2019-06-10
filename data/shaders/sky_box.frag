#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"

layout (location = 0) in vec3 inSampleDir;

layout (location = 0) out vec4 outFragColor0;

void main() 
{
	vec3 final = texture(RGBA16_1024_MIP_CUBE_SKYBOX, normalize(inSampleDir)).xyz;

	outFragColor0 = vec4(final, 1.0);
}