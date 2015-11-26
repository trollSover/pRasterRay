#include "App.hlsl"

struct INPUT
{
	uint dataPtr	: DATA_PTR;
	uint basePtr	: CHILD_BASE_PTR;
	uint parentPtr	: PARENT_PTR;
	int  children	: CHILD_BITMASK;
	int  level		: NODE_LEVEL;
};

struct OUTPUT
{
	float4 position : SV_POSITION;
	float3 wposition: POSITION;
	float2 uv		: TEXCOORD0;
};

OUTPUT main(INPUT input)
{
	OUTPUT output;
	float3 origin = float3(1, 1, 1);// MortonDecode(Voxels[input.dataPtr].morton);

	output.wposition = origin;
	output.position = mul( float4(origin,1), g_WVP);
	output.uv = float2(0, 0);

	return output;
}