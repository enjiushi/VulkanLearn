#pragma once

enum VertexAttribFlag
{
	VAFPosition,
	VAFNormal,
	VAFColor,
	VAFTexCoord,
	VAFTangent,
	VACount
};

enum VertexFormat
{
	VertexFormatNul = 0,
	VertexFormatP = (1 << VAFPosition),
	VertexFormatPN = (1 << VAFPosition) | (1 << VAFNormal),
	VertexFormatPTC = (1 << VAFPosition) | (1 << VAFTexCoord),
	VertexFormatPNTC = (1 << VAFPosition) | (1 << VAFNormal) | (1 << VAFTexCoord),
	VertexFormatPNTCT = (1 << VAFPosition) | (1 << VAFNormal) | (1 << VAFTexCoord) | (1 << VAFTangent)
};