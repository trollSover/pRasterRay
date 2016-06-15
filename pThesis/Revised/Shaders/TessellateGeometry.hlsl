
/*************************
--- INPUT/OUTPUT TYPES ---
**************************/

typedef struct 
{
	float4 position : POSITION;
	float4 color : COLOR;
} VS_INPUT;

typedef struct 
{
	float4 position : POSITION;
	float4 color : COLOR;
} VS_OUTPUT, HS_INPUT;

typedef struct 
{
	float4 position : POSITION;
	float4 color : COLOR;
} HS_OUTPUT, DS_INPUT;

typedef struct 
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
} DS_OUTPUT, PS_INPUT;

struct PS_OUTPUT
{
	float4	color	: SV_Target0;
	float4	normal	: SV_Target1;
	float	depth	: SV_Target2;
};

/********************
--- Vertex Shader ---
*********************/

VS_OUTPUT MainVS(VS_INPUT _input)
{
	VS_OUTPUT output;
	output.position = _input.position;
	output.color = _input.color;
	return output;
}

/******************
--- Hull Shader ---
*******************/

typedef struct 
{
	float edges[4]	: SV_TessFactor;
	float inside[2]	: SV_InsideTessFactor;

}	HS_CONSTANT_DATA_OUTPUT, DS_CONSTANT_DATA_INPUT;

HS_CONSTANT_DATA_OUTPUT HullConstantFunction(InputPatch<HS_INPUT, 4> _patch, uint patchId : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT output;
	output.edges[0] = 2;
	output.edges[1] = 2;
	output.edges[2] = 2;
	output.edges[3] = 2;

	output.inside[0] = 2;
	output.inside[1] = 2;

	return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HullConstantFunction")]
HS_OUTPUT MainHS(InputPatch<HS_INPUT, 4> _patch, uint _pointId : SV_OutputControlPointID, uint _patchId : SV_PrimitiveID)
{
	HS_OUTPUT output;
	output.position = _patch[_pointId].position;
	output.color = _patch[_pointId].color;
	return output;
}

/********************
--- Domain Shader ---
*********************/

[domain("quad")]
DS_OUTPUT MainDS(DS_CONSTANT_DATA_INPUT _input, float2 _uvCoord : SV_DomainLocation, const OutputPatch<DS_INPUT, 4> _patch, uint _patchId : SV_PrimitiveID)
{
	DS_OUTPUT output;
	output.position = _uvCoord.x * _patch[0].position + _uvCoord.y * _patch[1].position;// +_uvCoord.x * _patch[2].position + _uvCoord.y * _patch[3].position;
	output.position.w = 1;
	output.color = float4(_uvCoord.x, _uvCoord.y, 0, 1);
	/*float3 topMidPoint = lerp(_patch[0].position.xyz, _patch[3].position.xyz, _uvCoord.x);
	float3 bottomMidPoint = lerp(_patch[1].position.xyz, _patch[2].position.xyz, _uvCoord.x);
	output.position = float4(lerp(topMidPoint, bottomMidPoint, _uvCoord.y), 1);*/
	return output;
}



/*******************
--- Pixel Shader ---
********************/

PS_OUTPUT MainPS(PS_INPUT _input)
{
	PS_OUTPUT output;
	output.depth = _input.position.z / _input.position.w;
	output.normal = float4(1, 1, 1, 1);
	output.color = _input.color;

	return output;
}