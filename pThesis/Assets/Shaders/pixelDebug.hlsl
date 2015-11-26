#include "App.hlsl"

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 wposition : POSITION;
	float3 normal	: NORMAL;
	float4 color	: COLOR;
};

struct PS_OUTPUT
{
	float4 color		: SV_Target0;
	float4 normal		: SV_Target1;
	float  occlusion	: SV_Target2;
};

static const float4 ambientcolor = float4(0.5f, 0.5f, 0.5f, 1.0f);
static const float4 diffuseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
static const float3 lightDirection = float3(0.0f, -1.0f, 0.5f);


PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT psout;
	psout.color = input.color;
	psout.normal = float4(input.normal, 1);
	psout.occlusion = input.position.z / input.position.w;

	float4 colorCorrected = input.color / 256.f;
		colorCorrected.w = 1;
		float4 color = ambientcolor * colorCorrected;
	float	lightIntensity = saturate(dot(input.normal, -lightDirection));
	if (lightIntensity > 0.f)
		color += (diffuseColor * lightIntensity);
	psout.normal = saturate(color);

	return psout;
};