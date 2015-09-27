#include "App.hlsl"

static const float4 ScaleFactor;

struct INPUT
{
	float4 position : POSITION0;
	float2 texC		: TEXCOORD0;
};

struct OUTPUT
{
	float4 position	: POSITION0;
	float3 texC		: TEXCOORD0;
	float4 pos		: TEXCOORD1;
};

static const float2 quad[6] =
{
	float2(-1.0, -1.0),
	float2(1.0, -1.0),
	float2(-1.0, 1.0),
	float2(1.0, -1.0),
	float2(1.0, 1.0),
	float2(-1.0, 1.0)
};

static const float2 texC[6] =
{
	float2(0.0, 0.0),
	float2(0.0, 1.0),
	float2(1.0, 1.0),
	float2(0.0, 0.0),
	float2(1.0, 1.0),
	float2(1.0, 0.0),
};

OUTPUT main(INPUT input)
{
	OUTPUT output;

	output.position = mul(input.position * ScaleFactor, g_mWVP);
	output.texC		= input.position.xyz;
	output.pos		= output.position;

	return output;
}