#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inPrevClipSpacePos;
layout (location = 2) in vec3 inCurrClipSpacePos;

layout (location = 0) out vec4 outBGColorAndCoC;
layout (location = 1) out vec4 outBGMotionVec;

#include "uniform_layout.sh"
#include "global_parameters.sh"
#include "utilities.sh"

void main() 
{
	outBGColorAndCoC = vec4(vec3(0), CalculateCoC(perFrameData.nearFarAB.y));

	vec2 prevScreenCoord = inPrevClipSpacePos.xy / inPrevClipSpacePos.z;
	prevScreenCoord = prevScreenCoord * 0.5f + 0.5f;

	vec2 currScreenCoord = inCurrClipSpacePos.xy / inCurrClipSpacePos.z;
	currScreenCoord = currScreenCoord * 0.5f + 0.5f;

	outBGMotionVec = vec4(prevScreenCoord - currScreenCoord, 0.0f, 1.0f);
}