#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "uniform_layout.h"

layout (input_attachment_index = 0, set = 3, binding = 2) uniform subpassInput GBuffer0;
layout (input_attachment_index = 1, set = 3, binding = 3) uniform subpassInput GBuffer1;
layout (input_attachment_index = 2, set = 3, binding = 4) uniform subpassInput GBuffer2;
layout (input_attachment_index = 3, set = 3, binding = 5) uniform subpassInput DepthStencilBuffer;

layout (location = 0) in vec2 inUv;
layout (location = 1) in vec3 inViewRay;

layout (location = 0) out vec4 outFragColor;

struct GBufferVariables
{
	vec4 albedo_roughness;
	vec4 normal_ao;
	vec4 world_position;
};

// Hard-coded
float near = 1.0f;
float far = 2000.0f;

vec3 ReconstructPosition()
{
	float window_z = subpassLoad(DepthStencilBuffer).r;
	float eye_z = (far * near) / (window_z * (far - near) - far);

	vec3 viewRay = normalize(inViewRay);

	// Get cosine theta between view ray and camera direction
	float cos_viewRay_camDir = dot(viewRay, perFrameData.camDir.xyz);

	// "eye_z / cos_viewRay_camDir" is viewRay length between camera and surface point
	// NOTE: "eye_z" IS Z AXIS POSITION IN EYE SPACE, SO IT IS A MINUS VALUE, IF IT IS MULTIPLIED WITH A VECTOR, THEN WE SHOULD
	//       GET IT'S ABSOLUTE VALUE, I.E. DISTANCE TO CAMERA POSITION
	// DAMN THIS BUG HAUNTED ME FOR A DAY
	return viewRay * abs(eye_z) / cos_viewRay_camDir + perFrameData.camPos.xyz;
}


GBufferVariables UnpackGBuffers()
{
	GBufferVariables vars;

	vec4 gbuffer0 = subpassLoad(GBuffer0);
	vec4 gbuffer1 = subpassLoad(GBuffer1);
	vec4 gbuffer2 = subpassLoad(GBuffer2);

	vars.albedo_roughness.rgb = gbuffer1.rgb;
	vars.albedo_roughness.a = gbuffer0.a;
	vars.normal_ao.xy = gbuffer0.xy;
	vars.normal_ao.w = gbuffer2.a;

	vars.normal_ao.z = sqrt(1 - dot(gbuffer0.xy, gbuffer0.xy));

	vars.world_position = vec4(ReconstructPosition(), 1.0);

	return vars;
}

void main() 
{
	outFragColor = vec4(vec3(0), 1);

	GBufferVariables vars = UnpackGBuffers();

	//vec3 n = normal_ao.xyz;
	//vec3 v = normalize(inViewDir);
	//vec3 l = normalize(inLightDir);
	//vec3 h = normalize(l + v);

	//float NdotH = max(0.0f, dot(n, h));
	//float NdotL = max(0.0f, dot(n, l));
	//float NdotV = max(0.0f, dot(n, v));
	//float LdotH = max(0.0f, dot(l, h));

	if (length(vars.world_position.xyz) > 1000)	// Add this for debug purpose
		outFragColor.rgb = vec3(0);
	else
	{
		outFragColor.rgb = vars.world_position.xyz;
	}
}