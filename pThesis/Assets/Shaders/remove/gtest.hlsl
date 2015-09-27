#include "App.hlsl"
#include "MarchingCubes.hlsl"

struct Input
{
	float4 position : SV_POSITION;
	float3 wposition: POSITION;
	float2 uv		: TEXCOORD0;
};

struct Output
{
	float4 position : SV_POSITION;
	float3 wposition: POSITION0;
	float3 normal	: POSITION1;
	float2 uv		: TEXCOORD1;
};

struct GS_INPUT
{
	uint vertexId	: SV_VERTEXID;
};

struct Int64
{
	uint High;
	uint Low;
};

struct NODE
{
	Int64		m_data;
	Int64		m_base;
	bool		m_children[8];	// note: the child offset is based on order of appearance, not actual index

	float2		cache;
};

struct VOXEL
{
	Int64 morton;

	float2 data;
};

RWStructuredBuffer<NODE>	nodeBuffer;
RWStructuredBuffer<VOXEL>	voxelBuffer;

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

static const int N = 14;

[maxvertexcount(N)]
void main(point Input input[1], uint primID : SV_PrimitiveID, inout TriangleStream<Output> triStream)
{
	Output output;
	float4 origin = input[0].position;
	[unroll]
	for (int i = 0; i < N; ++i)
	{
		int index = ind[i];

		// position
		float4 offset = mul(float4(box[index],0), g_mWVP);

		output.position = origin + offset;
		
		// normal (not really)
		float3 wp			= box[index] - input[0].wposition;
		output.wposition	= wp;
		output.normal		= normalize(wp);

		// coord
		output.uv = input[0].uv;

		triStream.Append(output);
	}
};