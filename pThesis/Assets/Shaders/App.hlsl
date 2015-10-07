/*
--- APP.HLSL ---
*/

static const float3 LightDir = float3(0.5, -1, 0);
static const float3 Diffuse = float3(0.15, 0.15, 0.15);
static const float3 Ambient = float3(0.5, 0.5, 0.5);
static const float3 LightPos = float3(0, 512, 0);

cbuffer cbCamera : register(b0)
{
	float4x4 g_WVP;		// 64
	float4x4 g_View;		// 64
	float4x4 g_Projection;	// 64
	float4x4 g_World;	// 64

	float4x4 g_ViewInverse;
	float4x4 g_ProjectionInverse;
	float4x4 g_WVPInverse;
	float4x4 g_Rotation;

	float3	 g_cameraPos;	// 12
	float3	 g_cameraDir;	// 12
	float3	 g_right;			// 12
	float3	 g_up;			// 12


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
	uint gridLength;	// uniform length of cube
	uint rootDepth;		// depth N at root level where: { 0, 1, ..., N }
	uint rootIndex;		// root pointer for Node buffer
};						// = 16 bytes

cbuffer Constrictions : register(b3)
{
	float depthDivider;	// 4
	uint  NLoD;			// 4
	uint2 padding3;		// 8
};						// = 16 bytes


struct Volume_Color
{
	uint	morton;	// 4
	float4	color;	// 16
};					// = 20 bytes

typedef Volume_Color TVoxel;

//struct TVoxel
//{
//	uint	morton;	// 4
//	VData	data;	// 16
//};					// = 20 bytes

int ExtractNBits(const uint _bitMask, const uint _endIndex, const uint _Nbits)
{
	uint end = 32 - _endIndex * _Nbits;
	uint begin = end - _Nbits;
	uint mask = (1 << (end - begin)) - 1;
	return (_bitMask >> begin) & mask;
}

static const int	NODATA = 0;
static const uint	NOCHILD = 8;
static const int	LEAFMASK = -2004318072;

struct Node
{
	uint dataPtr;	// 4
	uint basePtr;	// 4	
	uint parentPtr;	// 4
	int children;	// 4
	int  level;		// 4
};					// = 20 bytes

struct Ray						// 48 bytes
{
	float3 origin;			// 12 bytes
	float3 direction;		// 12 bytes
};							// 24 bytes


RWStructuredBuffer<TVoxel>Voxels		: register(u0);
RWStructuredBuffer<Node>	Nodes		: register(u1);

//bool IsLeaf(Node _node)
//{
//	return _node.children == LEAFMASK;
//}

/* Decode 32-bit interleaved (xyz) morton */
float3 MortonDecode(uint morton)
{
	uint x, y, z;
	x = y = z = 0;
	for (uint i = 0; i < (32 / 3); ++i) {
		x |= ((morton & (uint(1) << uint((3 * i) + 0))) >> uint(((3 * i) + 0) - i));
		y |= ((morton & (uint(1) << uint((3 * i) + 1))) >> uint(((3 * i) + 1) - i));
		z |= ((morton & (uint(1) << uint((3 * i) + 2))) >> uint(((3 * i) + 2) - i));
	}
	return float3(x, y, z);
}

bool IntersectAABB(const float3 _rayOrigin, const float3 _rayDirection, out float _min, out float _max)
{
	/* source : http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
	slab intersection of AABB
	*/

	const float dim = 16.f;
	float near = -10000000.0f;
	float far = 10000000.0f;

	float t1, t2;
	t1 = t2 = 0;
	// parallel to plane ?
	for (int i = 0; i != 3; ++i)
	{
		if (_rayDirection[i] == 0 && (-dim > _rayOrigin[i] || dim < _rayOrigin[i]))
			return false;


		t1 = (-dim - _rayOrigin[i] / _rayDirection[i]);
		t2 = (dim - _rayOrigin[i] / _rayDirection[i]);

		if (t1 > t2)
		{
			float t = t1;
			t1 = t2;
			t2 = t;
		}

		if (t1 > near)
			near = t1;

		if (t2 < far)
			far = t2;

		if (near > far || far < 0.0f)
			return false;
	}

	_min = near;
	_max = far;

	return true;
}