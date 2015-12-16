/*
--- APP.HLSL ---
*/

static const float3 LightDir = float3(0.5, -1, 0);
static const float3 Diffuse = float3(0.15, 0.15, 0.15);
static const float3 Ambient = float3(0.5, 0.5, 0.5);
static const float3 LightPos = float3(0, 512, 0);

cbuffer cbCamera : register(b0)
{
	float4x4 g_WVP;					// 64
	float4x4 g_View;				// 64
	float4x4 g_Projection;			// 64
	float4x4 g_World;				// 64

	float4x4 g_ViewInverse;			// 64
	float4x4 g_ProjectionInverse;	// 64
	float4x4 g_WVPInverse;			// 64
	float4x4 g_Rotation;			// 64

	float3	 g_cameraPos;			// 12
	float a;
	float3	 g_cameraDir;			// 12
	float aa;
	float3	 g_right;				// 12
	float aaa;
	float3	 g_up;					// 12;
	float aaaa;
};							// = 512 bytes

cbuffer cbResolution : register(b1)
{
	uint  resWidth;	// 4
	uint  resHeight;// 4
	uint2 padding2;	// 8
};					// = 16 bytes

//cbuffer cbVoxel : register(b2)
//{
//	uint NVoxels;		// 4
//	uint NNodes;		// 4
//	uint NodeRootIndex;	// 4
//	uint padding1;		// 4
//};						// = 16 bytes

cbuffer cbVoxel : register(b2)
{
	uint numNodes;		// total amount of nodes
	int gridLength;		// uniform length of cube
	int rootDepth;		// depth N at root level where: { 0, 1, ..., N }
	int rootIndex;		// root pointer for Node buffer
};						// = 16 bytes

cbuffer CBOctreeMatrices : register(b3)
{
	float4x4	viewportToCamera;
	float4x4	cameraToOctree;
	/*float4x4	octreeToWorld;

	float4x4	worldToOctree;
	float4x4	OctreeToWorld;

	float4x4	octreeToViewPort;
	float4x4	viewportToOctree;

	float3		cameraPosition;*/
	float		pixelInOctree;
	float3		padding4;
};

cbuffer Constrictions : register(b4)
{
	float depthDivider;	// 4
	uint  NLoD;			// 4
	uint2 padding3;		// 8
};						// = 16 bytes

int ExtractNBits(const uint _bitMask, const uint _endIndex, const uint _Nbits)
{
	uint end = 32 - _endIndex * _Nbits;
	uint begin = end - _Nbits;
	uint mask = (1 << (end - begin)) - 1;
	return (_bitMask >> begin) & mask;
}

static const int	LEAFMASK = -2004318072;


