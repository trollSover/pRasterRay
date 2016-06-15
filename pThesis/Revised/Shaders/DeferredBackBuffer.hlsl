
Texture2D colorTexture  : register(t3);
Texture2D normalTexture	: register(t4);
Texture2D depthTexture	: register(t5);

SamplerState SampleTypePoint : register(s0);

struct VS_INPUT
{
	float4 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

VS_OUTPUT MainVS(VS_INPUT _input)
{
	VS_OUTPUT output;
	output.position = _input.position;
	output.texcoord = _input.texcoord;
	return output;
}

float4 MainPS(VS_OUTPUT _input) : SV_TARGET
{
	//return float4(1, 1, 1, 1);
	float3 normal = normalTexture.Sample(SampleTypePoint, _input.texcoord).xyz;
	float4 color = colorTexture.Sample(SampleTypePoint, _input.texcoord);

	float4 finalColor = color;

	return finalColor;
	
}