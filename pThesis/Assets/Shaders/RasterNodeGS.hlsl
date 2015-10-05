#include "App.hlsl"

struct INPUT
{
	float4 position : SV_POSITION;
	float3 wposition: POSITION;
	float2 uv		: TEXCOORD0;
};

struct OUTPUT
{
	float4 position : SV_POSITION;
	float3 wposition: POSITION0;
	float3 normal	: POSITION1;
	float2 uv		: TEXCOORD0;
};

static const int ind[14] =
{
	1,
	0,
	2,	// front
	4,	// left
	6,
	7,	// back
	2,	// top
	3,
	1,	//front
	7,	//right
	5,
	4,	// back
	1,	// bottom
	0
};

static const float UNIT = 0.5;
static const float3 box[8] =
{
	float3(-UNIT, -UNIT, -UNIT),	//0
	float3(UNIT, -UNIT, -UNIT),		//1
	float3(-UNIT, UNIT, -UNIT),		//2
	float3(UNIT, UNIT, -UNIT),		//3
	float3(-UNIT, -UNIT, UNIT),		//4
	float3(UNIT, -UNIT, UNIT),		//5
	float3(-UNIT, UNIT, UNIT),		//6
	float3(UNIT, UNIT, UNIT)		//7
};

[maxvertexcount(14)]
void main(point INPUT input[1], inout TriangleStream<OUTPUT> triStream)
{
	OUTPUT output;

	float4 origin = input[0].position;

	[unroll]
	for (int i = 0; i < 14; ++i)
	{
		int index = ind[i];

		float4 offset = mul(float4(box[index], 0), g_WVP);
		output.position = origin + offset;

		float3 wp = box[index] - input[0].wposition;
		output.wposition = wp;
		output.normal = normalize(wp);
		output.uv = input[0].uv;
		triStream.Append(output);
	}
}