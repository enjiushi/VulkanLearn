#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

layout (location = 0) out vec3 outSampleDir;

layout (set = 0, binding = 0) uniform GlobalUniforms
{
	mat4 projection;
	mat4 vulkanNDC;
	mat4 PN;
}globalUniforms;

layout (set = 1, binding = 0) uniform PerFrameUniforms
{
	mat4 view;
	mat4 VPN;
	vec4 camPos;
}perFrameUniforms;

struct PerObjectData
{
	mat4 model;
	mat4 MVPN;
};

layout (set = 2, binding = 0) buffer PerObjectBuffer
{
	PerObjectData perObjectData[];
};

void main() 
{
	mat4 view = perFrameUniforms.view;
	view[3] = vec4(vec3(0.0), 1.0);
	vec4 pos = globalUniforms.vulkanNDC * globalUniforms.projection * view * vec4(inPos.xyz, 1.0);
	gl_Position = pos.xyww;

	outSampleDir = normalize(vec3(inPos.x, -inPos.y, inPos.z));
}
