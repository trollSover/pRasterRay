#include "ConstantBuffers.hlsl"

/* Work/Thread distribution */
#ifndef THREAD_COUNT_X
#define THREAD_COUNT_X 1
#endif
#ifndef THREAD_COUNT_Y
#define THREAD_COUNT_Y 1
#endif
#ifndef WORK_SIZE_Y
#define WORK_SIZE_Y 1
#endif
#ifndef WORK_SIZE_X
#define WORK_SIZE_X 1
#endif

/* Raycast limits */
#ifndef STACK_LIMIT
#define STACK_LIMIT 1
#endif
#ifndef ITR_LIMIT
#define ITR_LIMIT 1
#endif

/* In retrospect the Node struct could be optimized by dropping basePtr and simply keep track of the index
externally since basePtr is essentially it's index value in the buffer. */

/* --- Node ---
#dataPtr:	Voxel buffer index for potential voxel data. Supports leaf and LoD data and allows for minimizing duplicate data in the voxel buffer.
#basePtr:	Node buffer index of self. Used for calculating offsets for child nodes, ie. child index = basePtr + child_N offset 
#children:	Child mask. First 8 bits show children (set bit = child, unset bit = empty). Following 3*8 bits represents child offsets corresponding to the first 8 bits */
struct Node
{
	uint dataPtr;	// 4
	uint basePtr;	// 4	
	int children;	// 4
};					// = 12 bytes

/* --- TVoxel ---
T data goes in here - in this case a simple color & normal data */
struct TVoxel
{
	float4	color;	// 16
	float3  normal;	// 12

};					// = 28 bytes

/* --- Ray ---
#origin:	starting position of ray
#direction: looking direction of ray*/
struct Ray
{
	float3 origin;			// 12 bytes
	float3 direction;		// 12 bytes	
};							// 24 bytes

// structured buffers
RWStructuredBuffer<Node>	Nodes		: register(u0);
RWStructuredBuffer<TVoxel>	Voxels		: register(u1);

// read/write buffers
RWTexture2D<float>  readBuffer  : register(u2);	// z-buffer 
RWTexture2D<float4> writeBuffer : register(u3);	// color buffer
RWTexture2D<float4> normalDest : register(u4);	// normal buffer

#define BLACK		float4(0,0,0,1);
#define WHITE		float4(1,1,1,1);
#define RED			float4(1,0,0,1);
#define GREEN		float4(0,1,0,1);
#define BLUE		float4(0,0,1,1);
#define PURPLE		float4(1,0,1,1);
#define YELLOW		float4(1,1,0,1);
#define GREENBLUE	float4(0,1,1,1);
#define GREY		float4(0.5,0.5,0.5,1);

/* constants */
cbuffer CBOctree : register(cb2)
{
	//uint numNodes;
	uint	gridLength;
	uint	maxDepth;
	uint	rootIndex;
	float3	rootOrigin;
	uint2	padcbo;
};

cbuffer cbImmutable
{
	/* colors corresponding to octree child offset position */
	static const float4 colors[] = {
		float4(1, 0, 0, 1),			// 0	: RED
		float4(0, 1, 0, 1),			// 1	: GREEN
		float4(0, 0, 1, 1),			// 2	: BLUE
		float4(1, 1, 0, 1),			// 3	: YELLOW
		float4(1, 0, 1, 1),			// 4	: PURPLE
		float4(0, 1, 1, 1),			// 5	: BABY BLUE
		float4(0.75, 0.75, 0.75, 1),// 6	: GREY
		float4(1, 1, 1, 1)			// 7	: WHITE
	};

	/* octree child offset position */
	
	static const float3 pos[] =
	{
		float3(0, 0, 0),		// 0	: RED
		float3(1, 0, 0),		// 1	: GREEN
		float3(0, 1, 0),		// 2	: BLUE
		float3(1, 1, 0),		// 3	: YELLOW
		float3(0, 0, -1),		// 4	: PURPLE
		float3(1, 0, -1),		// 5	: BABY BLUE
		float3(0, 1, -1),		// 6	: GREY
		float3(1, 1, -1),		// 7	: WHITE
	};
	
	/*
	static const float3 pos[] =
	{
		float3(-0.5, -0.5, 0),		// 0	: RED
		float3(0.5, -0.5, 0),		// 1	: GREEN
		float3(-0.5, 0.5, 0),		// 2	: BLUE
		float3(0.5, 0.5, 0),		// 3	: YELLOW
		float3(-0.5, -0.5, 1),		// 4	: PURPLE
		float3(0.5, -0.5, 1),		// 5	: BABY BLUE
		float3(-0.5, 0.5, 1),		// 6	: GREY
		float3(0.5, 0.5, 1),		// 7	: WHITE
	};
	*/
	//static const float3 wmin = mul(float4(0, 0, 0, 1), g_ViewInverse).xyz;
	//static const float3 wmax = mul(float4(gridLength, gridLength, gridLength, 1), g_ViewInverse).xyz;

	static const float4 ambientcolor = float4(0.05f, 0.05f, 0.05f, 1.0f);
	static const float4 diffuseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	static const float3 lightDirection = float3(0.0f, -1.0f, 0.f);
};

inline bool Intersect(Ray r, in float3 _min, in float3 _max, out float t0, out float t1)
{
	float3 invR = 1.0 / r.direction;
	float3 tbot = invR * (_min - r.origin);
	float3 ttop = invR * (_max - r.origin);
	float3 tmin = min(ttop, tbot);
	float3 tmax = max(ttop, tbot);
	float2 t	= max(tmin.xx, tmin.yz);

	t0	= max(t.x, t.y);
	t	= min(tmax.xx, tmax.yz);
	t1	= min(t.x, t.y);

	return t0 <= t1;
}

/* --- Check if node has child at specified index ---
#param:
*_nodePtr: node index address
*_index: node child index (by morton order)
#return:
*true IF child indicator bit set
#false IF else */
bool HasChildAtIndex(const in uint _nodePtr, const in uint _index)
{
	return 1 == ((Nodes[_nodePtr].children >> _index) & ((1 << 1) - 1));
}

/* --- Check the amount of children of a node ---
#param:
*_nodePtr: node index address
#return:
*all set child indicator bits in descriptor (maximum: 8) */
uint CountChildren(const in uint _nodePtr)
{
	// count set bits in the first 8 bits of the child-descriptor
	return countbits(Nodes[_nodePtr].children & ((1 << 8) - 1));
}

/* --- Get child pointer address ---
#param:
*_nodePtr: node index address
*_index: node child index (by morton order)
#return:
*base index + child offset */
uint GetChildAddress(const in uint _nodePtr, const in uint _index)
{
	// return Node::basePtr + 3 offset bits @ Node::children[8 + 3 * _index];
	return Nodes[_nodePtr].basePtr + (Nodes[_nodePtr].children >> (8 + 3 * _index) & ((1 << 3) - 1));
}

/* --- Check if node is leaf ---
#param:
*_nodePtr: node index address
#return:
*true IF no children
*false IF else */
bool IsLeaf(const in uint _nodePtr)
{
	// compare zero to bitwise AND of the first 8 bits in child-descriptor
	//return  0 == (Nodes[_nodePtr].children & ((1 << 8) - 1));

	return  0 == (Nodes[_nodePtr].children & ((1 << 8) - 1)) && Nodes[_nodePtr].dataPtr != 0;
}

bool IsNull(const in uint _nodePtr)
{
	return IsLeaf(_nodePtr) && (Nodes[_nodePtr].dataPtr == 0);
}

#pragma warning( disable : 4714 )	// due to CAST_STACK_DEPTH we reach the limit of temp registers, 
									// will negatively affect performance - disable warning for now

void CTraverse(in Ray _ray, in uint2 pixel)
{
	// stack RAM usage(B) = sizeof(Stack) * resWidth * resHeight

	int index = STACK_LIMIT - 1;

	int itr = 0;

	struct CStack
	{
		Node	node;
		int		childmask;		
		float3	origin;
		float	scale;
		float	tmax;
	} stack[STACK_LIMIT];

	Node currentNode = Nodes[rootIndex];

	stack[index].node		= currentNode;								// node reference at stack index
	stack[index].childmask  = currentNode.children& ((1 << 8) - 1);		// bitfield to mask off traversed children
	stack[index].origin		= rootOrigin;								// root coordinates
	
	stack[index].scale		= 1.f / ((float)maxDepth + 1);
	float4 COLOR			= PURPLE;// float4(0, 0, 0, 0);
	bool raycastVoxel		= false;

	float tmin, tmax;

	const float DIM = gridLength * 2;
	const float hd = DIM / 2;

	float3 omax = stack[index].origin + pos[7] * (1 << (maxDepth - 1));

	// ray doesn't intersect root
	if (!Intersect(_ray, stack[index].origin, omax, tmin, tmax))
	{
		//writeBuffer[pixel.xy] = YELLOW;
		return;
	}

	// only negative ray intersect root
	if (tmin >= tmax)
	{
		writeBuffer[pixel.xy] = RED;
		return;
	}

	//if (tmax <= 0)
	//{
	//	writeBuffer[pixel.xy] = GREEN;
	//	return;	// prevent false positives on negative ray directions
	//}

	// max limit of node intersection
	float fmax = tmax;

	stack[index].tmax = max(tmin,tmax);
	
	while (index < STACK_LIMIT)
	{
		
		itr++;
		// IF stack index OR itr out of bounds - terminate traversal
		if (index < 0 || itr > ITR_LIMIT)
		{
			
			writeBuffer[pixel.xy] = PURPLE;
			break;
		}

		// Current node has no children	
		if (currentNode.children == 0 && stack[index].scale <= 1.f)
		//if (currentNode.children == 0)
		{
			
			// terminating leaf - record voxel data
			if (currentNode.dataPtr)
			{
				//fmax = min(stack[index].tmax, fmax);
				
				TVoxel voxel = Voxels[currentNode.dataPtr];

				fmax = stack[index].tmax;
				//normalDest[pixel.xy] = float4( normalize(voxel.normal),1);
				normalDest[pixel.xy] = float4(voxel.normal, 1);
				COLOR = voxel.color / 256.f;
				//COLOR = voxel.color * 256.f;
				//COLOR.xyz = ((float)itr / (float)ITR_LIMIT) * index;
				//COLOR = WHITE;
				COLOR.w = 1;
				//COLOR = RED;
				raycastVoxel = true;
			}
			
		}

		// current nodes children have all been masked off
		if (stack[index].childmask == 0)
		{
			index++;							// pop stack
			currentNode = stack[index].node;	// current node = stack top
			continue;
		}
		
		// get next child in line & mask it out
		int childIdx			= firstbitlow(stack[index].childmask);			// can child traversal be optimized by direction dependent bitreversal?

		//_ray.direction.z > 0 ? childIdx = firstbitlow(stack[index].childmask) : firstbithigh(stack[index].childmask);

		stack[index].childmask	= stack[index].childmask & ~(1 << childIdx);	// mask single bit at childIdx

		// AABB by child offset from origin
		float  scale = stack[index].scale;
		float  hdim  = scale * 1 * DIM;							// half dimension of current node level
		float3 vmin  = stack[index].origin + pos[childIdx] * hdim;	// parent origin - child offset
		float3 vmax  = vmin + pos[7] * hdim;						// parent origin + child dim

		// Check if child intersects
		if (Intersect(_ray, vmin, vmax, tmin, tmax))
		{
			// node origin farther away than already recorded terminal node
			if (tmin >= fmax)
				continue;

			//if (tmax <= 0) continue;	// prevent false positives on negative ray directions

			// fetch child and replace current node
			currentNode = Nodes[currentNode.basePtr + (currentNode.children >> (8 + 3 * childIdx) & ((1 << 3) - 1))];	// Node::basePtr + 3 offset bits @ Node::children[8 + 3 * childIdx];

			// push stack
			index--;
			stack[index].node		= currentNode;					
			stack[index].childmask	= currentNode.children& ((1 << 8) - 1);	// fresh childmask
			stack[index].origin		= vmin;
			stack[index].scale		= scale * 0.5;
			stack[index].tmax		= tmin;
		}
	}

	//GroupMemoryBarrierWithGroupSync();
	//if (raycastVoxel)
	{
		uint2 texel = pixel;// uint2(pixel.x + 1, pixel.y + 1);
		writeBuffer[texel.xy] = COLOR;
	}
}

float4 TransformRay(float4 _ray)
{
	_ray = mul(_ray, g_WorldViewProjectionInverse);
	_ray /= _ray.w;
	_ray -= float4(g_cameraPos, 0);
	return _ray;
}

#pragma warning( disable : 3556 )	// complain about using int div instead of uint div
 //{32 * 40, 45 * 16} 
//[numthreads(32, 16, 1)]
[numthreads(THREAD_COUNT_X, THREAD_COUNT_Y, 1)]
void MainCS(uint3 threadId : SV_DispatchThreadID, uint3 groupId : SV_GroupThreadID)
{
	// Naive depth discard
	//note:	* Since the raycasting operates on faraway geometry
	//we can discard any pixel already written to sincen
	//any rasterized object (closer geometry) are by
	//default	prioritized.
	if (readBuffer[threadId.xy].r > 0)
	{
		return;
	}

	float N = WORK_SIZE_X * WORK_SIZE_Y;
	float invN = 1.f / N;
	//float x = float(2.f * threadId.x + 1.f - N) * invN;
	//float y = -float(2.f * threadId.y + 1.f - N) * invN;
	//float y = -float(2.f * (float)threadId.y + 1.f - (float)g_resolutionHeight) * (1.f / (float)g_resolutionHeight) + 1;
	//float x = float(2.f * (float)threadId.x + 1.f - (float)g_resolutionWidth)  * (1.f / (float)g_resolutionWidth) + 1;
	float y = -(float)threadId.y * (1.f / ((float)g_resolutionHeight * 0.5f)) + 1.f;
	float x = (float)threadId.x * (1.f / ((float)g_resolutionWidth  * 0.5f)) - 1.f;
	//float y = -(float)threadId.y * (1.f / ((float)g_resolutionWidth * 0.5f)) + 1.f;
	//float x = (float)threadId.x * (1.f / ((float)g_resolutionWidth  * 0.5f)) - 1.f;

	//x = (float)threadId.x * 1.f / (float)g_resolutionWidth * 2 - 1;
	//y = -(float)threadId.y * 1.f / (float)g_resolutionHeight * 2 + 1;
	float z = 1;
	// Create new ray from the camera position to the pixel position
	Ray ray;
	//float4 aux = (mul(float4(0, 0, 0, 1.f), g_ViewInverse));
	float4 aux = mul(float4(0, 0, 0, 1.f), g_ViewInverse);
	ray.origin = aux.xyz / aux.w;

	float4 pixelPosition = mul(float4(x, y, z, 1.f), g_ViewInverse);
	pixelPosition.xyz /= pixelPosition.w;

	ray.direction = normalize(pixelPosition.xyz - ray.origin);

	CTraverse(ray, threadId.xy);
}

