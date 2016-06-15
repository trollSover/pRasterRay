#include "ConstantBuffers.hlsl"

/************************
--- INPUT/OUTPUT YPES ---
*************************/

typedef struct
{
	float3 position : POSITION;
	float3 normal	: NORMAL;
	float4 color	: COLOR;
} VS_INPUT;

typedef struct
{
	float4 position : SV_POSITION;
	float3 normal	: NORMAL;
	float4 color	: COLOR;
} VS_OUTPUT, PS_INPUT;

typedef struct
{
	float4 color	: SV_target0;
	float4 normal	: SV_Target1;
	float  depth	: SV_Target2;
} PS_OUTPUT;

/********************
--- Vertex Shader ---
*********************/

VS_OUTPUT MainVS(VS_INPUT _input)
{
	VS_OUTPUT output;
	output.position	= mul(float4(_input.position, 1), g_WorldViewProjection);
	output.normal	= _input.normal;
	output.color	= _input.color;

	return output;
}

/*******************
--- Pixel Shader ---
********************/

PS_OUTPUT MainPS(PS_INPUT _input)
{
	PS_OUTPUT output;
	output.color	= _input.color / 256.f;
	output.color.w	= 1;
	output.normal	= float4(_input.normal, 1);
	output.depth	= _input.position.z / _input.position.w;

	return output;
}