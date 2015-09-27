#include "App.hlsl"

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float3 wposition: POSITION0;
	float3 normal	: POSITION1;
	float2 uv		: TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 color : SV_Target0;
	float4 normal: SV_Target1;
	float  depth : SV_Target2;
};


static const float3 SpecularColor = float3(1, 1, 1);
static const float SpecularPow = 0.5;
float4 Saturate(float3 _light, float3 _normal)
{
	return saturate(dot(_light, _normal));
}

float3 calcBP(float3 N, float3 L, float3 H)
{
	float3 Ia = 0.1f * float3(1, 1, 1);
		float3 Id = 0.75f * saturate(dot(N, L));
		return Ia + Id;
}

static const float3 cTable[4] =
{
	float3(0.85, 0.67, 0.33),	// sand
	float3(.2, 0.6, 0.17),		// grass
	float3(0.55, 0.55, 0.55),	// rock
	float3(1, 1, 1)		// snow
};

float3 getHColor(float _height)
{
	float3 hcolor = float3(1, 1, 1);

	if (_height > -5) hcolor = cTable[0];
	else if (_height > -80) hcolor = cTable[1];
	else if (_height > -200) hcolor = cTable[2];
	else hcolor = cTable[3];

	return hcolor;
}

PS_OUTPUT main(PS_INPUT input)
{
	float3 tex = getHColor(input.wposition.y);
	float3 n = normalize(input.normal);
	float3 V = normalize(g_cameraPos - input.wposition.xyz);
	float3 h = normalize(LightDir + V);

	//float4 color = float4(saturate(tex * calcBP(n, LightDir, h)), 1);
	float4 color = float4(saturate(Ambient + tex * calcBP(n, LightDir, h)), 1);
	//float4 color = float4(Ambient * float3(input.uv,1) + Diffuse * Saturate(-LightDir, input.normal).xyz, 1);

	PS_OUTPUT psout;
	psout.color  = float4(1,0,0,1);
	psout.normal = float4(1,0,0,1);
	psout.depth = input.position.z / input.position.w;

	return psout;
	//return color;
};