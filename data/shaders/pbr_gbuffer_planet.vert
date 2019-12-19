#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inBarycentricCoord;

layout (location = 1) in vec3 inTriangleVertexA;
layout (location = 2) in vec3 inTriangleVertexB;
layout (location = 3) in vec3 inTriangleVertexC;

layout (location = 0) out vec2 outUv;
layout (location = 1) out vec3 outCSNormal;
layout (location = 2) out vec3 outCSPosition;
layout (location = 3) noperspective out vec2 outScreenPosition;
layout (location = 4) out vec3 outPrevCSPosition;
layout (location = 5) out vec4 outDistToEdge;

#include "uniform_layout.sh"

void main() 
{
	int indirectIndex = GetIndirectIndex(gl_DrawID, 0);

	int perObjectIndex = objectDataIndex[indirectIndex].perObjectIndex;


	vec3 position = inTriangleVertexA * inBarycentricCoord.x + inTriangleVertexB * inBarycentricCoord.y + inTriangleVertexC * inBarycentricCoord.z;

	int vertexID = gl_VertexIndex % 3;
	if (vertexID == 0)
	{
		outDistToEdge.xyz = vec3(0, 0, 1);
	}
	else if (vertexID == 1)
	{
		outDistToEdge.xyz = vec3(1, 0, 0);
	}
	else
	{
		outDistToEdge.xyz = vec3(0, 1, 0);
	}

	// w represents the edge of the patch rather than sub triangles
	outDistToEdge.w = min(min(inBarycentricCoord.x, inBarycentricCoord.y), inBarycentricCoord.z);
	outDistToEdge.w = 1.0f - step(0.01, outDistToEdge.w);

	position = normalize(position);

	gl_Position = perObjectData[perObjectIndex].MVP * vec4(position, 1.0);

	vec3 localNormal = normalize(position);

	outCSNormal = normalize(vec3(perObjectData[perObjectIndex].MV * vec4(localNormal, 0.0)));
	outCSPosition = (perObjectData[perObjectIndex].MV * vec4(position, 1.0)).xyz;
	outPrevCSPosition = (perObjectData[perObjectIndex].prevMV * vec4(position, 1.0)).xyz;
	outScreenPosition = gl_Position.xy / gl_Position.w;

	outUv = vec2(0, 0);
	outUv.t = 1.0 - outUv.t;
}
