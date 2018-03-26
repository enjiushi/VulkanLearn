#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (input_attachment_index = 0, set = 3, binding = 2) uniform subpassInput GBuffer0;
layout (input_attachment_index = 1, set = 3, binding = 3) uniform subpassInput GBuffer1;
layout (input_attachment_index = 2, set = 3, binding = 4) uniform subpassInput GBuffer2;
layout (input_attachment_index = 3, set = 3, binding = 5) uniform subpassInput DepthStencilBuffer;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = vec4(subpassLoad(GBuffer0).xy, 0, 1);
}