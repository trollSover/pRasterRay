

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

VS_OUTPUT main(uint vertexId: SV_VERTEXID)
{
	VS_OUTPUT vsout;
	vsout.texcoord = float2((vertexId << 1) & 2, vertexId & 2);
	vsout.position = float4(vsout.texcoord * float2(2.f, -2.f) + float2(-1.f, 1.f), 0.f, 1.f);

	return vsout;
}