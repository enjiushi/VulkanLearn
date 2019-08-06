#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

layout (location = 0) out vec3 outSampleDir;

#include "uniform_layout.sh"

void main() 
{
	mat4 view = perFrameData.view;
	view[3] = vec4(vec3(0.0), 1.0);
	vec4 pos = globalData.projection * view * vec4(inPos.xyz, 1.0);
	gl_Position = pos.xyww;

	outSampleDir = normalize(vec3(inPos.x, -inPos.y, inPos.z));
}
