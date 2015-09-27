#include "App.hlsl"

struct PS_INPUT
{
	float4 position : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 color = float4(1,1,1,1);

	return color;
};