#include "App.hlsl"

struct VS_INPUT
{
	float3 position : POSITION;
	float2 uv		: TEXCOORD0;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD0;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	output.position.w = 1.0f;
	output.position = mul(float4(input.position, 1), g_mWVP);
	output.uv = input.uv;
	return output;
}