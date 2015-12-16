#include "Shaders/App.hlsl"

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

RWStructuredBuffer<TVoxel>	Voxels		: register(u0);
RWStructuredBuffer<Node>	Nodes		: register(u1);

// read/write buffers
RWTexture2D<float>  readBuffer  : register(u4);	// z-buffer 
RWTexture2D<float4> writeBuffer : register(u5);	// color buffer

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
		float3(0, 0, 1),		// 4	: PURPLE
		float3(1, 0, 1),		// 5	: BABY BLUE
		float3(0, 1, 1),		// 6	: GREY
		float3(1, 1, 1),		// 7	: WHITE
	};

	//static const float3 wmin = mul(float4(0, 0, 0, 1), g_ViewInverse).xyz;
	//static const float3 wmax = mul(float4(gridLength, gridLength, gridLength, 1), g_ViewInverse).xyz;

	static const float4 ambientcolor = float4(0.05f, 0.05f, 0.05f, 1.0f);
	static const float4 diffuseColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	static const float3 lightDirection = float3(0.0f, -1.0f, 0.f);
};

bool Intersect(Ray r, in float3 _min, in float3 _max, out float t0, out float t1)
{
	float3 invR = 1.0 / r.direction;
	float3 tbot = invR * (_min - r.origin);
	float3 ttop = invR * (_max - r.origin);
	float3 tmin = min(ttop, tbot);
	float3 tmax = max(ttop, tbot);
	float2 t = max(tmin.xx, tmin.yz);
	t0 = max(t.x, t.y);
	t = min(tmax.xx, tmax.yz);
	t1 = min(t.x, t.y);
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


#pragma warning( disable : 4714 )	// due to CAST_STACK_DEPTH we reach the limit of temp registers, will negatively affect performance - disable warning for now

//struct Stack
//{
//	int		nodeptr;
//	int		idx;
//	float	scale;
//	float3	origin;
//};

//void BTraverse(in Ray _ray, in uint2 pixel)
//{
//	const int dim = gridLength;
//
//	// stack RAM usage(B) = sizeof(Stack) * resWidth * resHeight
//
//
//	int stackIndex = STACK_LIMIT - 1;
//	Stack stack[STACK_LIMIT];
//
//	stack[stackIndex].nodeptr = rootIndex;
//	stack[stackIndex].idx = 0;
//	stack[stackIndex].scale = 1;
//	stack[stackIndex].origin = float3(0, 0, 0);// float3(dim, dim, dim) * -0.5f;
//	//writeBuffer[pixel.xy] = float4(244.f / 256.f, 164.f / 256.f, 96.f / 256.f, 1);
//
//	float mn = 0;
//	float mx = 0;
//	//if (!Intersect(_ray, stack[stackIndex].origin, stack[stackIndex].origin + pos[7] * dim, mn, mx))
//	//{
//	//	writeBuffer[pixel.xy] = PURPLE;
//	//	return;
//	//}
//	//if (mn > 0)
//	//{
//	//	writeBuffer[pixel.xy] = RED;
//	//	return;
//	//}
//	float lim = 0;
//	float tlim = 0;
//	int itr = 0;
//
//	//while (stackIndex < STACK_LIMIT)
//	while (stackIndex < STACK_LIMIT)
//	{
//		itr++;
//
//		if (stackIndex < 0)
//		{
//			//writeBuffer[pixel.xy] = RED;
//			break;
//		}
//
//		if (itr > ITR_LIMIT)
//		{
//			//writeBuffer[pixel.xy] = YELLOW;
//			break;
//		}
//
//		int		node = stack[stackIndex].nodeptr;
//		int		idx = stack[stackIndex].idx;
//		float	scale = stack[stackIndex].scale;
//		float3	origin = stack[stackIndex].origin;
//
//			// does the current node intersect the ray?
//			float3 omin = origin;
//			float3 omax = origin + pos[7] * (dim * scale);
//			float  tmin = 0;
//		float tmax = 0;
//
//
//		if (idx < 8)
//		{
//			stack[stackIndex].idx++;	// advance in advance
//		}
//		else
//		{
//			stackIndex++;
//			continue;
//		}
//
//		if (IsNull(node))
//		{
//			stackIndex++;
//			continue;
//		}
//
//		if (IsLeaf(node))
//			//if (Nodes[node].children == 0)
//		{
//			//if (tlim > lim)
//			//{
//			//	stackIndex--;
//			//	continue;
//			//}
//			int voxelptr = Nodes[node].dataPtr;
//			float4 color = Voxels[voxelptr].color / 256.f;
//				float3 nor = Voxels[voxelptr].normal;
//				//float4 color = Voxels[voxelptr].color * 256.f;
//				color.w = 1;
//
//
//
//			float4 col = color * ambientcolor;
//				float	lightIntensity = saturate(dot(nor, lightDirection));
//			if (lightIntensity > 0.f)
//				col += (diffuseColor * lightIntensity);
//
//			
//			float ci = (float)itr / (float)ITR_LIMIT;// *(float)stackIndex / (float)STACK_LIMIT;
//			//ci = 1 - omax.y / dim * -1;
//			writeBuffer[pixel.xy] = float4(ci, ci, ci, 1) * color;
//
//			writeBuffer[pixel.xy] = color;
//			break;
//
//			stackIndex++;
//			continue;
//
//		}
//
//		/*float3 tomin = omin  + pos[idx] * (dim * scale * 0.5f);
//		float3 tomax = tomin + pos[7]	* (dim * scale * 0.5f);
//		float tvmin, tvmax;*/
//		omin = omin + pos[idx] * (dim * scale *0.5f);
//		omax = omin + pos[7] * (dim * scale * 0.5f);
//
//		if (HasChildAtIndex(node, idx))
//		{
//			float ci = (float)itr / (float)ITR_LIMIT * (float)stackIndex / (float)STACK_LIMIT;
//			//writeBuffer[pixel.xy] = float4(ci, ci, ci, 1);
//			//writeBuffer[pixel.xy] = colors[idx] * c;
//
//			// do node child[idx] intersect?
//			//if (Intersect(_ray, tomin, tomax, tvmin, tvmax))
//			if (Intersect(_ray, omin, omax, tmin, tmax))
//			{
//				float atmin = max(tmin, 0);
//				float atmax = min(tmin, tmax);
//				if (atmin <= atmax)
//				{
//					//if (min(tmin,tmax) < lim)
//					{
//						
//						// child intersect - ADD
//						stackIndex--;
//						stack[stackIndex].nodeptr = GetChildAddress(node, idx);
//						stack[stackIndex].idx = 0;
//						stack[stackIndex].scale = scale * 0.5f;
//						stack[stackIndex].origin = omin;
//
//						continue;
//					}
//				}
//			}
//		}
//
//		if (idx > 7)
//			stackIndex++;
//	}
//	float ci = (float)stackIndex / (float)STACK_LIMIT; //(float)itr / (float)ITR_LIMIT;// *
//	//writeBuffer[pixel.xy] = float4(ci, ci, ci, 1);
//}


//struct RCResult
//{
//	int itr;
//	int stackIndex;
//	int voxelptr;
//};

void CTraverse(in Ray _ray, in uint2 pixel)
{
	// stack RAM usage(B) = sizeof(Stack) * resWidth * resHeight

	int index = STACK_LIMIT - 1;
	int itr = 0;

	struct CStack
	{
		Node node;
		int childmask;		
		float3 origin;
		float scale;
		float tmax;
	} stack[STACK_LIMIT];

	Node currentNode = Nodes[rootIndex];

	stack[index].node		= currentNode;								// node reference at stack index
	stack[index].childmask  = currentNode.children& ((1 << 8) - 1);		// bitfield to mask off traversed children
	stack[index].origin = float3(0, 0, 0);	//float3(1, 1, 1) *(float)gridLength -0.5;// 				// origo
	stack[index].scale = 1.f;
	float4 COLOR = WHITE;// float4(0, 0, 0, 0);
	bool raycastVoxel = false;

	float tmin, tmax;

	const float DIM = gridLength;

	int octant = (_ray.direction.x > 0) | (_ray.direction.y > 0) * 2 | (_ray.direction.z > 0) * 4;

	// ray doesn't intersect root
	if (!Intersect(_ray, stack[index].origin, stack[index].origin + pos[7] * (1 << (rootDepth - 1)), tmin, tmax))
	{
		return;
	}
	// only negative ray intersect root
	if (tmin >= tmax)
		return;

	// max limit of node intersection
	float fmax = tmax;

	stack[index].tmax = max(tmin,tmax);

	//while (stackIndex < STACK_LIMIT)
	while (index < STACK_LIMIT)
	{
		itr++;

		// IF stack index OR itr out of bounds - terminate traversal
		if (index < 0 || itr > ITR_LIMIT)
		{
			//COLOR = YELLOW;
			break;
		}

		// Current node has no children	
		if (currentNode.children == 0)
		{
			// terminating leaf - record voxel data
			if (currentNode.dataPtr)
			{
				//fmax = min(stack[index].tmax, fmax);
				
				TVoxel voxel = Voxels[currentNode.dataPtr];

				fmax = stack[index].tmax;

				COLOR = voxel.color / 256.f;
				//COLOR = voxel.color;
				COLOR.w = 1;
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
		stack[index].childmask	= stack[index].childmask & ~(1 << childIdx);	// mask bit at childIdx

		// AABB by child offset from origin
		float scale = stack[index].scale;
		float hdim  = scale * 0.25 * DIM;							// half dimension of current node level
		float3 vmin = stack[index].origin + pos[childIdx] * hdim;	// parent origin - child offset
		float3 vmax = vmin + pos[7] * hdim;							// parent origin + child dim

		// Check if child intersects
		if (Intersect(_ray, vmin, vmax, tmin, tmax))
		{
			if (tmin >= tmax)
				continue;

			// node origin farther away than already recorded terminal node
			if (tmin > fmax)
				continue;

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
	
	GroupMemoryBarrierWithGroupSync();
	if (raycastVoxel)
		writeBuffer[pixel.xy] = COLOR;
}

#pragma warning( disable : 3556 )	// complain about using int div instead of uint div
 //{32 * 40, 45 * 16} 
[numthreads(32, 16, 1)]
//[numthreads(THREAD_COUNT_X, THREAD_COUNT_Y, 1)]
void main(uint3 threadId : SV_DispatchThreadID, uint3 groupId : SV_GroupThreadID)
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

	//float y = float(2.f * (float)threadId.y + 1.f - (float)resHeight) * (1.f / (float)resHeight) + 1;
	//float x = float(2.f * (float)threadId.x + 1.f - (float)resWidth)  * (1.f / (float)resWidth) + 1;
	float y = -(float)threadId.y * (1.f / ((float)resHeight * 0.5f)) + 1.f;
	float x =  (float)threadId.x * (1.f / ((float)resWidth  * 0.5f)) - 1.f;
	float z = g_up.z;	// camera focal length

	// Create new ray from the camera position to the pixel position
	Ray ray;
	float4 aux = (mul(float4(0, 0, 0, 1.f), g_ViewInverse));
	ray.origin = aux.xyz / aux.w;
	ray.origin = g_cameraPos;

	float3 pixelPosition = mul(float4(x, y, z, 1.f), g_ViewInverse).xyz;
	ray.direction = normalize(pixelPosition - ray.origin);

	GroupMemoryBarrierWithGroupSync();
	//BTraverse(ray, threadId.xy);
	CTraverse(ray, threadId.xy);
}

//[numthreads(THREAD_COUNT_X, THREAD_COUNT_Y, 1)]
//void main(uint3 threadId : SV_DispatchThreadID, uint3 groupId : SV_GroupThreadID, uint groupIndex : SV_GroupIndex)
//{
//	unsigned int2 tId = threadId.xy;
//	tId.x = WORK_SIZE_X * groupId.x;
//	tId.y = WORK_SIZE_Y * groupId.y;
//	unsigned int id = groupIndex; // groupId.x + groupId.y;
//
//	RCResult res;
//	res.itr = 0;
//	res.voxelptr = 0;
//	res.stackIndex = 0;
//	writeBuffer[threadId.xy] = RED;
//	[unroll]
//	for (unsigned int j = 0; j < WORK_SIZE_Y; ++j)
//	{
//		tId.y = WORK_SIZE_Y * groupId.y + j;
//
//		[unroll]
//		for (unsigned int i = 0; i < WORK_SIZE_X; ++i)
//		{
//			tId.x = WORK_SIZE_X * groupId.x + i;
//
//			if (readBuffer[tId.xy].r > 0)
//			{
//				continue;
//			}
//
//			//writeBuffer[tId.xy] = colors[id % 8];
//
//			float y = float(2.f * tId.y + 1.f - resHeight) * (1.f / (float)resHeight) + 1;
//			float x = float(2.f * tId.x + 1.f - resWidth)  * (1.f / (float)resWidth) + 1;
//			//float y = (float)threadId.y * (1.f / (resHeight * 0.5f)) + 1.f;
//			//float x = (float)threadId.x * (1.f / (resWidth  * 0.5f)) - 1.f;
//			float z = 1.0f;
//
//			// Create new ray from the camera position to the pixel position
//			Ray ray;
//			float4 aux = (mul(float4(0, 0, 0, 1.f), g_ViewInverse));
//			ray.origin = aux.xyz / aux.w;
//			ray.origin_sz = 0;
//			ray.direction_sz = 0;
//
//			// create ray direction from pixelposition in world space
//			float3 pixelPosition = mul(float4(x, y, z, 1.f), g_ViewInverse).xyz;
//			ray.direction = normalize(pixelPosition - ray.origin);
//			ray.origin_sz = 0;
//			ray.direction_sz = 0;
//
//			BTraverse(ray, tId);
//		}
//	}
//
//
//
//	//writeBuffer[threadId.xy] = RED;// bdata[id];
//	//for (unsigned int i = 0; i < WORK_SIZE_X; ++i)
//	//{
//	//	color[i] = colors[groupIndex % 8];
//	//}
//
//	//for (unsigned int j = 0; j < WORK_SIZE_X; ++j)
//	//{
//	//	writeBuffer[tId.xy + uint2(j,0)] = color[j];
//	//}
//	
//}