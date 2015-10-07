#include "Shaders/App.hlsl"

// output buffer - in this case the backbuffer
RWTexture2D<float>  readBuffer  : register(u4);
RWTexture2D<float4> writeBuffer : register(u5);

static const float maxDist = sqrt(2.0);
static const int numSamples = 128;
static const float stepSize = maxDist / float(numSamples);


static const float4 colors[] = {
	float4(1, 0, 0, 1),
	float4(0, 1, 0, 1),
	float4(0, 0, 1, 1),
	float4(1, 1, 0, 1),
	float4(1, 0, 1, 1),
	float4(0, 1, 1, 1),
	float4(0, 0, 0, 1),
	float4(1, 1, 1, 1)
};

struct AABB {
	float3 Min;
	float3 Max;
};

AABB getBox(float3 _min, float3 _max)
{
	AABB aabb;
	aabb.Min = _min;
	aabb.Max = _max;
	return aabb;
}

bool IntersectBox(Ray r, AABB aabb, out float t0, out float t1)
{
	float3 invR = 1.0 / r.direction;
		float3 tbot = invR * (aabb.Min - r.origin);
		float3 ttop = invR * (aabb.Max - r.origin);
		float3 tmin = min(ttop, tbot);
		float3 tmax = max(ttop, tbot);
		float2 t = max(tmin.xx, tmin.yz);
		t0 = max(t.x, t.y);
	t = min(tmax.xx, tmax.yz);
	t1 = min(t.x, t.y);
	return t0 <= t1;
}

bool hitbox(Ray r, float3 m1, float3 m2, out float tmin, out float tmax)
{
	float tymin, tymax, tzmin, tzmax;
	float flag = 1.0;

	if (r.direction.x >= 0)
	{
		tmin = (m1.x - r.origin.x) / r.direction.x;
		tmax = (m2.x - r.origin.x) / r.direction.x;
	}
	else
	{
		tmin = (m2.x - r.origin.x) / r.direction.x;
		tmax = (m1.x - r.origin.x) / r.direction.x;
	}
	if (r.direction.y >= 0)
	{
		tymin = (m1.y - r.origin.y) / r.direction.y;
		tymax = (m2.y - r.origin.y) / r.direction.y;
	}
	else
	{
		tymin = (m2.y - r.origin.y) / r.direction.y;
		tymax = (m1.y - r.origin.y) / r.direction.y;
	}

	if ((tmin > tymax) || (tymin > tmax)) flag = -1.0;
	if (tymin > tmin) tmin = tymin;
	if (tymax < tmax) tmax = tymax;

	if (r.direction.z >= 0)
	{
		tzmin = (m1.z - r.origin.z) / r.direction.z;
		tzmax = (m2.z - r.origin.z) / r.direction.z;
	}
	else
	{
		tzmin = (m2.z - r.origin.z) / r.direction.z;
		tzmax = (m1.z - r.origin.z) / r.direction.z;
	}
	if ((tmin > tzmax) || (tzmin > tmax)) flag = -1.0;
	if (tzmin > tmin) tmin = tzmin;
	if (tzmax < tmax) tmax = tzmax;

	return (flag > 0);
}

/* Check if node has child at specified index
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

/* Check the amount of children of a node
#param:
*_nodePtr: node index address
#return:
*all set child indicator bits in descriptor (maximum: 8) */
uint CountChildren(const in uint _nodePtr)
{
	// count set bits in the first 8 bits of the child-descriptor
	return countbits(Nodes[_nodePtr].children & ((1 << 8) - 1));
}

/* Get child pointer address
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

/* Check if node is leaf
#param:
*_nodePtr: node index address
#return:
*true IF no children
*false IF else */
bool IsLeaf(const in uint _nodePtr)
{
	// compare zero to bitwise AND of the first 8 bits in child-descriptor
	return  0 == (Nodes[_nodePtr].children & ((1 << 8) - 1));
}

bool Intersect(Ray _ray, AABB _box, out float3 _intersectionPoint)
{
	float3 tmin = (_box.Min - _ray.origin) / _ray.direction;
		float3 tmax = (_box.Max - _ray.origin) / _ray.direction;

		_intersectionPoint = min(tmin, tmax);

	float3 real_min = min(tmin, tmax);
		float3 real_max = max(tmin, tmax);

		float minmax = min(min(real_max.x, real_max.y), real_max.z);
	float maxmin = max(max(real_min.x, real_min.y), real_min.z);

	return (minmax >= maxmin);
}

float EuclideanDistance(in float3 _ro, in float3 _no)
{
	return sqrt((_ro.x - _no.x) * (_ro.x - _no.x) +
		(_ro.y - _no.y) * (_ro.y - _no.y) +
		(_ro.z - _no.z) * (_ro.z - _no.z));
}

// svo iterative traversal - not working as of now, needs to be re-written
void Traverse(Ray _ray, uint2 pixelId)
{
	uint nodePtr = rootIndex;
	AABB box;
	box.Min = float3(0, 0, 0);
	box.Max = float3(gridLength, gridLength, gridLength);

	float dim = pow(2, Nodes[nodePtr].level);

	float tnear, tfar;
	uint rColorId = Nodes[rootIndex].level;
	//	uint colorId = Nodes[nodePtr].level;

	int count = 0;


	float fdim[] =
	{
		1,
		2,
		4,
		8,
		16,
		32,
		64,
		128
	};

	float3 minoff[] =
	{
		float3(0, 0, 0),
		float3(1, 0, 0),
		float3(0, 1, 0),
		float3(1, 1, 0),
		float3(0, 0, 1),
		float3(1, 0, 1),
		float3(0, 1, 1),
		float3(1, 1, 1),
	};

	float minDist = 100000;

	do
	{
		//if (Nodes[nodePtr].level < 1)	// remove
		//	break;

		if (IntersectBox(_ray, box, tnear, tfar))			// we intersect the node
		{
			if (IsLeaf(nodePtr))							// leaf - possible render
			{
				if (tnear >= 0.f)							// render
				{
					//colorId = Nodes[nodePtr].level;

					uint colorId = Nodes[nodePtr].level;	// color cube by depth

					float3 p = box.Max - box.Min / 2;

						float d = 0;
					if (d = EuclideanDistance(_ray.origin, p) < minDist)
					{
						minDist = d;
						writeBuffer[pixelId] = colors[colorId];

					}

					//if(!DrawCube(_ray, box, pixelId))
					//writeBuffer[pixelId] = colors[colorId];

					//return;
				}
				else
				{

				}
			}
			else											// not a leaf - check children
			{
				dim = pow(2, Nodes[nodePtr].level - 1);
				float hdim = dim * 0.5;
				for (int i = 0; i < 8; ++i)
				{
					if (HasChildAtIndex(nodePtr, i))
					{
						nodePtr = GetChildAddress(nodePtr, i);	// current node is child[i]

						box.Min = box.Min + minoff[i] * float3(dim, dim, dim);
						box.Max = box.Min + float3(hdim, hdim, hdim);


						break;
					}
				}
			}
		}
		else
		{
			if (nodePtr == rootIndex)	// root didn't intersect - just leave
				return;

			nodePtr = Nodes[nodePtr].parentPtr;
		}

		if (count > 64)
			break;

		count++;

	} while (nodePtr != rootIndex);
}

static const float3 mins[8] =
{
	float3(0, 0, 0),
	float3(256, 0, 0),
	float3(0, 256, 0),
	float3(256, 256, 0),
	float3(0, 0, 256),
	float3(256, 0, 256),
	float3(0, 256, 256),
	float3(256, 256, 256)
};
static const float3 maxs[8] =
{
	float3(256, 256, 256),
	float3(512, 256, 256),
	float3(256, 512, 256),
	float3(512, 512, 256),
	float3(256, 256, 512),
	float3(512, 256, 512),
	float3(256, 512, 512),
	float3(512, 512, 512)
};

// {32 * 40, 45 * 16} 
[numthreads(32, 16, 1)]
void main(uint3 threadId : SV_DispatchThreadID, uint3 groupId : SV_GroupThreadID)
{
	/* Naive depth discard
	note:	* Since the raycasting operates on faraway geometry
	we can discard any pixel already written to since
	any rasterized object (closer geometry) are by
	default	prioritized.*/
	if (readBuffer[threadId.xy].r > 0)
	{
		return;
	}

	float y = -float(2.f * threadId.y + 1.f - resWidth) * (1.f / (float)resWidth);
	float x = float(2.f * threadId.x + 1.f - resWidth)  * (1.f / (float)resWidth);
	float z = 1.0f;

	// Create new ray from the camera position to the pixel position
	Ray ray;
	float4 aux = (mul(float4(0, 0, 0, 1.f), g_ViewInverse));
		ray.origin = aux.xyz / aux.w;

	float tnear, tfar;

	// create ray direction from pixelposition in world space
	float3 pixelPosition = mul(float4(x, y, z, 1.f), g_ViewInverse).xyz;
		ray.direction = normalize(pixelPosition - ray.origin);

	uint nodeptr = rootIndex;

	int hlen = gridLength;// / 2;
	AABB aabb;
	aabb.Min = float3(-0, -0, -0);
	aabb.Max = float3(hlen, hlen, hlen);
	//aabb.Max = mul(float4(aabb.Max, 1), g_ViewInverse).xyz;
	//aabb.Min = mul(float4(aabb.Min, 1), g_ViewInverse).xyz;

	// check if the ray intersect the root AABB
	// if not: quit
	// else continue with further checks
	if (IntersectBox(ray, aabb, tnear, tfar))
	{
		//if (tnear >= 1)
		//	writeBuffer[threadId.xy] = float4(0.125, 0.125, 0.125, 1);
		int N = CountChildren(nodeptr);
		writeBuffer[threadId.xy] = colors[N];
	}

	// SVO traversal goes in here
	//int level = Nodes[nodeptr].level;

	// debug rendering
	for (int i = 0; i < 8; ++i)
	{
		if (HasChildAtIndex(nodeptr, i))
		{
			AABB box;
			box.Min = mins[i];
			box.Max = maxs[i];
			float tn, tf;
			if (IntersectBox(ray, box, tn, tf))
			{
				if (tn >= 0.f)
				{
					writeBuffer[threadId.xy] = colors[i];	// this does not take fragment distance into account and override according to order processed
				}
			}
			//writeBuffer[threadId.xy] = float4(1, 0, 0, 1);
			//return;
		}
		else
		{
			//writeBuffer[threadId.xy] = float4(1, 1, 1, 1);
		}
	}


	return;
}
