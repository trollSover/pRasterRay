#include "App.hlsl"

#pragma warning(disable: 4000)
#pragma warning(disable: 3203)
static const float3 GRIDSIZE = float3(1, 1, 1);

struct INPUT
{
	float4 position	: POSITION0;
	float3 texC		: TECOORD0;
	float4 pos		: TEXCOORD1;
};

Texture2D frontTexture;
Texture2D backTexture;
Texture3D volumeTexture;

SamplerState sample2D;
SamplerState sample3D;

static const float3 StepSize = float3(1, 1, 1);
static const int Iterations = 10;

float4 main(INPUT input) : SV_TARGET
{
	float2 texC = input.pos.xy /= input.pos.w;
	texC.x =  0.5f * texC.x + 0.5f;
	texC.y = -0.5f * texC.y + 0.5f;

	float3 front = frontTexture.Sample(sample2D, texC).xyz;
	float3 back = backTexture.Sample(sample2D, texC).xyz;
	
	float3 dir = normalize(back - front);
	float4 pos = float4(front, 0);
	
	float4 dst = float4(0, 0, 0, 0);
	float4 src = 0;
	
	float value = 0;

	float3 step = dir * StepSize;

	for (int i = 0; i < Iterations; ++i)
	{
		pos.w = 0;
		value = volumeTexture.SampleLevel(sample3D, pos.xyz, 0).r;

		src = (float4)value;
		src.a *= 0.5f;
		src.rgb *= src.a;
		dst = (1.0f - dst.a) * src + dst;

		if (dst.a >= 0.95f)
			break;

		pos.xyz += step;

		if (pos.x > 1.0f || pos.y > 1.0f || pos.z > 1.0f)
			break;

	}

	return dst;
}