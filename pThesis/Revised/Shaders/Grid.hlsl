#include "ConstantBuffers.hlsl"

/*************************
--- INPUT/OUTPUT TYPES ---
**************************/

cbuffer CBRotatePlane : register(b3)
{
	float4x4 rotateTranslateOnAxis;	// translation added so all planes align in a common direction
	float4 color;
};

typedef struct
{
	float3 position : POSITION;
} VS_INPUT;

typedef struct
{
	float4 position : SV_POSITION;
} VS_OUTPUT, PS_INPUT;

typedef struct
{
	float4 color	: SV_Target0;
	float4 normal	: SV_Target1;
	float  depth	: SV_Target2;
} PS_OUTPUT;

/********************
--- Vertex Shader ---
*********************/

VS_OUTPUT MainVS(VS_INPUT _input)
{
	VS_OUTPUT output;
	//output.position = mul(float4(_input.position, 1), mul(rotateTranslateOnAxis, g_WorldViewProjection));
	output.position = mul(float4(_input.position, 1), g_WorldViewProjection);
	return output;
}

/*******************
--- Pixel Shader ---
********************/

PS_OUTPUT MainPS(PS_INPUT _input)
{
	PS_OUTPUT output;
	output.color = color;// float4(1, 1, 1, 1);
	output.normal	= float4(0, 1, 0, 1);
	output.depth	= _input.position.z / _input.position.w;

	return output;
}
