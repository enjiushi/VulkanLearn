#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"
#include "global_parameters.sh"

layout (set = 3, binding = 3) uniform sampler2D motionTileMax[3];

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outTileNeighborMax;

void main() 
{
	vec2 du = vec2(1.0f / globalData.motionTileWindowSize.z, 0.0f);
	vec2 dv = vec2(0.0f, 1.0f / globalData.motionTileWindowSize.w);

	vec2 maxMotion = vec2(0);
	float maxLength = 0;

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y < 1; y++)
		{
			vec2 motionVec = texture(motionTileMax[frameIndex], inUv + x * du + y * dv).rg;
			float len = dot(motionVec, motionVec);
			if (maxLength < len)
			{
				maxLength = len;
				maxMotion = motionVec;
			}
		}
	}

	outTileNeighborMax = vec4(maxMotion, 0.0f, 0.0f);
}