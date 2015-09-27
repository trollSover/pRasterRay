#include "App.hlsl"

struct VS_INPUT
{
	float3 position : POSITION;
	float2 uv		: TEXCOORD0;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float3 wposition: POSITION;
	float2 uv		: TEXCOORD0;
};

PS_INPUT main(VS_INPUT input, uint vertexId : VertexID)
{
	PS_INPUT output;

	output.position		= mul(float4(input.position, 1), g_mWVP);
	output.wposition	= input.position;
	output.uv			= input.uv;
	int mod = vertexId % 4;
	output.uv = normalize(float2(mod, mod));
	return output;
};