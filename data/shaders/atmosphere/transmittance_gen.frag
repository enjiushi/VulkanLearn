#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "functions.sh"

layout(location = 0) out vec3 transmittance;

void main() 
{
	transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture(ATMOSPHERE, gl_FragCoord.xy);
}