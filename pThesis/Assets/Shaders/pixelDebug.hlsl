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

float4 Saturate(float3 _light, float3 _normal)
{
	return saturate(dot(_light, _normal));
}

float3 calcBP(float3 N, float3 L, float3 H)
{
	float3 Ia = 0.5f * float3(1, 1, 1);
	float3 Id = 0.5f * saturate(dot(N, L));
	float3 Is = 10.25f * pow(saturate(dot(N, H)), 0.1);
	return Ia + Id +Is;
}

PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT psout;
	psout.color = input.color;
	psout.normal = float4(input.normal, 1);
	psout.occlusion = input.position.z / input.position.w;

	return psout;

	//float3 V = normalize(g_cameraPos - input.wposition.xyz);
	//float3 h = normalize(LightDir + V);

	////float4 color = float4( saturate(Ambient + tex * calcBP(n, -LightDir, h)), 1);
	//float4 color = float4(Ambient * input.color.xyz + Diffuse * Saturate(LightDir, input.normal).xyz, 1);

	//return color;
};