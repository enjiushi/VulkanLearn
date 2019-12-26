#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.sh"
#include "global_parameters.sh"

layout (set = 3, binding = 3) uniform sampler2D motionVector[3];

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outTileMax;

void main() 
{
	vec2 base = inUv + globalData.gameWindowSize.zw * (0.5f - 0.5f * globalData.motionTileWindowSize.xy);
	vec2 du = vec2(globalData.gameWindowSize.z, 0.0f);
	vec2 dv = vec2(0.0f, globalData.gameWindowSize.w);

	vec2 maxMotion = vec2(0);
	float maxLength = 0;

	for (int x = 0; x < int(globalData.motionTileWindowSize.x); x++)
	{
		for (int y = 0; y < int(globalData.motionTileWindowSize.y); y++)
		{
			vec2 motionVec = texture(motionVector[frameIndex], base + x * du + y * dv).rg;
			float len = dot(motionVec, motionVec);
			if (maxLength < len)
			{
				maxLength = len;
				maxMotion = motionVec;
			}
		}
	}

	outTileMax = vec4(maxMotion, 0.0f, 0.0f);
}