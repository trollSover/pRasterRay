#include "App.hlsl"

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal	: NORMAL;
	float4 color	: COLOR;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 wposition : POSITION;
	float3 normal	: NORMAL;
	float4 color	: COLOR;
};

PS_INPUT main(VS_INPUT input, uint vertexId : SV_VERTEXID)
{
	PS_INPUT output;

	output.position		= mul(float4(input.position, 1), g_WVP);
	output.wposition	= float4(input.position,1);
	output.normal		= normalize(input.normal);
	output.color		= input.color;

	return output;
};