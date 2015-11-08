

Texture2D colorTexture  : register(t4);
Texture2D normalTexture	: register(t3);
Texture2D depthTexture	: register(t5);

SamplerState SampleTypePoint : register(s0);

struct PSINPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

float4 main(PSINPUT input) : SV_TARGET
{
	//float4 color = depthTexture.Sample(SampleTypePoint, input.texcoord) * 100.f;
	//color.w = 1;
	//float4 color	= normalTexture.Sample(SampleTypePoint, input.texcoord);
	float4 color = colorTexture.Sample(SampleTypePoint, input.texcoord);

	return color;
}