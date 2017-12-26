#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 3, binding = 1) uniform samplerCube envTex;

#include "uniform_layout.h"

layout (location = 0) in vec3 inSampleDir;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 final = texture(RGBA16_1024_MIP_CUBE_SKYBOX, normalize(inSampleDir)).xyz;
	
	final = Uncharted2Tonemap(final * globalData.GEW.y);
	final = final * (1.0 / Uncharted2Tonemap(vec3(globalData.GEW.z)));
	final = pow(final, vec3(globalData.GEW.x));

	outFragColor = vec4(final, 1.0);
}