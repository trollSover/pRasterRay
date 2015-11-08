#include "Shaders/App.hlsl"

// output buffer - in this case the backbuffer
RWTexture2D<float>  readBuffer  : register(u4);
RWTexture2D<float4> writeBuffer : register(u5);


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
	float2 t	= max(tmin.xx, tmin.yz);
	t0			= max(t.x, t.y);
	t			= min(tmax.xx, tmax.yz);
	t1			= min(t.x, t.y);
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

//float EuclideanDistance(in float3 _ro, in float3 _no)
//{
//	return sqrt((_ro.x - _no.x) * (_ro.x - _no.x) +
//		(_ro.y - _no.y) * (_ro.y - _no.y) +
//		(_ro.z - _no.z) * (_ro.z - _no.z));
//}

#pragma warning( disable : 4714 )	// due to CAST_STACK_DEPTH we reach the limit of temp registers, will negatively affect performance - disable warning for now

#define CAST_STACK_DEPTH 23
#define MAX_ITERATIONS  10000


void Traverse(inout Ray _ray, uint3 threadId)
{
	/*
	Desc:
		tmin, tmax: initialized to the intersection of the ray and root
		h:			threshold value for tmax to prevent unnecessary writes to stack
		idx:		current node. initialized by comparing tmin against tx,ty,tz at octree origin
		pos, scale:	current cube desc
		t: ?
		tv: ?
		tc: ?
		stack: ?
		smax: ?
	INITIALIZE:
	1. (tmin,tmax) <- (0,1)
	2. t' <- project cube(root, ray)
	3. t <- intersect(t,t')
	4. h <- t'max
	5: parent <- root
	6: idx <- select child(root, ray, tmin)
	7: (pos, scale) <- child cube(root,idx)

	8: WHILE NOT terminated DO															while(terminated == false)
	9:		tc <- project cube(pos, scale, ray)											{
	10:		IF voxel exist AND tmin <= tmax THEN											if(voxel exist && tmin <= tmax)
																							{
	INTERSECT:																					if(voxel is small enough) return tmin				// RETURN Tmin
	11:			IF voxel is small enough THEN RETURN tmin										tc = intersect(tc,t)
	12:			tc <- intersect(tc, t)
	... // contouring - not something we do														if(tvmin <= tvmax)
																								{
	17:			IF tvmin <= tvmax THEN																if(voxel is leaf) return tvmin					// RETURN TVmin
																									if(tcmax < h)
	PUSH:																								stack[scale] <- (parent,tmax)
	18:				IF voxel is a leaf THEN RETURN tvmin											
	19:				IF tcmax < h THEN stack[scale] <- (parent, tmax)								h = tcmax
	20:				h <- tcmax																		parent <- find child descriptor(parent,tmax)
	21:				parent <- find child descriptor(parent, idx)									idx = select child(pos, scale, ray, tvmin)
	22:				idx <- select child(pos, scale, ray, tvmin)										t = tv
	23:				t <- tv																			(pos, scale) <- child cube(pos, scale, idx)
	24:				(pos, scale) <- child cube(pos, scale, idx)										continue;
	25:				CONTINUE																	}
																							}
	26:			ENDIF
	27:		ENDIF																			oldpos = pos
																							(pos, idx) <- step along ray(pos, scale, ray)
	ADVANCE:																				tmin = tcmax
	28:		oldpos <- pos
	29:		(pos, idx) <- step along ray(pos, scale, ray)									if(idx update disagrees with ray)
	30:		tmin <- tcmax																	{
																								scale = highest differing bit(pos, oldpos)
	31:		IF idx update disagrees with ray THEN												if(scale >= smax) return miss						// RETURN MISS

	POP:																						(parent, tmax) <- stack[scale]
	32:			scale <- highest differing bit(pos, oldpos)										pos <- round position(pos, scale)
	33:			IF scale >= smax THEN RETURN miss												idx = extract child slot index(pos, scale)
	34:			(parent, tmax) <- stack[scale]													h = 0
	35:			pos <- round position(pos, scale)											}
	36:			idx <- extract child slot index(pos, scale)								}
	37:			h <- 0

	38:		ENDIF
	39:	END WHILE

	*/
	
	int		nodeStack[CAST_STACK_DEPTH + 1];
	float	tmaxStack[CAST_STACK_DEPTH + 1]; 

	//nodeStack[CAST_STACK_DEPTH] = rootIndex;
	
	const float epsilon = exp2(-CAST_STACK_DEPTH);

	// avoid division by zero on small ray direction
	// note: copysign is not present in hlsl
	if (abs(_ray.direction.x) < epsilon) _ray.direction.x = (epsilon * sign(_ray.direction.x));
	if (abs(_ray.direction.y) < epsilon) _ray.direction.y = (epsilon * sign(_ray.direction.y));
	if (abs(_ray.direction.z) < epsilon) _ray.direction.z = (epsilon * sign(_ray.direction.z));

	const int INVALID_CHILD_DESCRIPTOR = 0;	// no bits set

	// precompute coefficients of t
	float tx_cf = 1.f / -abs(_ray.direction.x);
	float ty_cf = 1.f / -abs(_ray.direction.y);
	float tz_cf = 1.f / -abs(_ray.direction.z);

	float tx_bs = tx_cf * _ray.origin.x;
	float ty_bs = ty_cf * _ray.origin.y;
	float tz_bs = tz_cf * _ray.origin.z;

	// octant mask
	int octant_mask = 7;
	if (_ray.direction.x > 0.f) octant_mask ^= 1, tx_bs = 3.f * tx_cf - tx_bs;
	if (_ray.direction.y > 0.f) octant_mask ^= 2, ty_bs = 3.f * ty_cf - ty_bs;
	if (_ray.direction.z > 0.f) octant_mask ^= 4, tz_bs = 3.f * tz_cf - tz_bs;

	// init span of t-values
	float t_min = max(max(2.f * tx_cf - tx_bs, 2.f * ty_cf - ty_bs), 2.f * tz_cf - tz_bs);
	float t_max = min(min(tx_cf - tx_bs, ty_cf - ty_bs), tz_cf - tz_bs);
	float h		= t_max;
	t_min		= max(t_min, 0.f);
	t_max		= min(t_max, 1.f);

	// init current node
	int parent			= rootIndex;
	int descriptor		= INVALID_CHILD_DESCRIPTOR;
	int idx				= 0;
	float3 pos			= float3(1.f, 1.f, 1.f);
	int scale			= CAST_STACK_DEPTH - 1;
	float scale_exp2	= 0.5f;

	if (1.5f * tx_cf - tx_bs > t_min) idx ^= 1, pos.x = 1.5f;
	if (1.5f * ty_cf - ty_bs > t_min) idx ^= 2, pos.y = 1.5f;
	if (1.5f * tz_cf - tz_bs > t_min) idx ^= 4, pos.z = 1.5f;
	
	int itr = 0;
	

	float ray_dir_sz =_ray.direction_sz;
	float ray_pos_sz =_ray.origin_sz;

	//if (t_min <= t_max)
	//{
	//	writeBuffer[threadId.xy] = float4(1.f, 0.f, 0.f, 1.f);
	//}
	//else
	//{
	//	writeBuffer[threadId.xy] = float4(0.f, 1.f, 0.f, 1.f);
	//}

	//writeBuffer[threadId.xy] = float4(0.75f, 0.5f, 0.25f, 1.f);
	
	// traverse nodes along ray within the octree
	while (scale < CAST_STACK_DEPTH)
	{
		itr++;
		if (itr > MAX_ITERATIONS)	// iteration ceiling reached - abort
			break;

		if (idx < 0 || idx > 7 || parent < 0 || parent > (int)rootIndex)	// debug sanity check
		{
			writeBuffer[threadId.xy] = float4(0.75, 0.5, 0.25, 1);
			return;
		}

		// get child descriptor of parent unless already set
		if (descriptor == INVALID_CHILD_DESCRIPTOR)
		{
			descriptor = Nodes[parent].children;
		}

		// determine max t-value of cube
		float tx_corner = pos.x * tx_cf - tx_bs;
		float ty_corner = pos.y * ty_cf - ty_bs;
		float tz_corner = pos.z * tz_cf - tz_bs;
		float tc_max = min(min(tx_corner, ty_corner), tz_corner);

		int child_shift = idx ^ octant_mask;
		int child_masks = descriptor << child_shift;

		// if ray hits root this will always be true on the first iteration
		//if ((child_masks & 0x8000) != 0 && t_min <= t_max)
		if (t_min <= t_max)
		{
			//float c = color_shade_step * itr;
			writeBuffer[threadId.xy] = float4(0, 1, 0, 1);

			//if (tc_max * ray.dir_sz + ray_orig_sz >= scale_exp2)
			//	break; // at t_min

			if (tc_max * ray_dir_sz + ray_pos_sz >= scale_exp2)
			{
				writeBuffer[threadId.xy] = float4(0, 1, 1, 1);
				return;
			}

			// INTERSECT
			float tv_max	= min(t_max, tc_max);
			float fhalf		= scale_exp2 * 0.5f;
			float tx_center = fhalf * tx_cf + tx_corner;
			float ty_center = fhalf * ty_cf + ty_corner;
			float tz_center = fhalf * tz_cf + tz_corner;

			// Descend to the first child if the resulting t-span is non-empty.
			if (t_min <= tv_max)
			{
				if ((child_masks & 0x0080) == 0)
				{
					writeBuffer[threadId.xy] = float4(1, 1, 1, 1);	// red
					return; // at t_min (overridden with tv_min).
				}

				

				// PUSH
				if (tc_max < h)
				{
					// stack write
					nodeStack[scale] = parent;
					tmaxStack[scale] = t_max;
					
				}
				
				h = tc_max;
				// Find child descriptor corresponding to the current voxel.
				//int offset = (unsigned int)descriptor >> 17;	// child pointer
				int offset = (unsigned int)descriptor;
				//if ((descriptor & 0x10000) != 0)	// far - not our concern
				//	break;

				offset += popc8(child_masks & 0x7F);

				// Select child voxel that the ray enters first.
				idx = 0;
				scale--;
				scale_exp2 = fhalf;
				
				if (tx_center > t_min) idx ^= 1, pos.x += scale_exp2;
				if (ty_center > t_min) idx ^= 2, pos.y += scale_exp2;
				if (tz_center > t_min) idx ^= 4, pos.z += scale_exp2;

				// Update active t-span and invalidate cached child descriptor.
				t_max = tv_max;
				descriptor = 0;
				continue;
			}
		}

		
		// ADVANCE
		// Step along the ray

		int step_mask = 0;
		if (tx_corner <= tc_max) step_mask ^= 1, pos.x -= scale_exp2;
		if (ty_corner <= tc_max) step_mask ^= 2, pos.y -= scale_exp2;
		if (tz_corner <= tc_max) step_mask ^= 4, pos.z -= scale_exp2;

		// Update active t-span and flip bits of the child slot index.

		t_min = tc_max;
		idx ^= step_mask;

		// Proceed with pop if the bit flips disagree with the ray direction.
		if ((idx & step_mask) != 0)
		{
			//writeBuffer[threadId.xy] = float4(0, 1, 0, 1);	// green
			// POP

			unsigned int differing_bits = 0;
			if ((step_mask & 1) != 0) differing_bits |= asint(pos.x) ^ asint(pos.x + scale_exp2);
			if ((step_mask & 2) != 0) differing_bits |= asint(pos.y) ^ asint(pos.y + scale_exp2);
			if ((step_mask & 4) != 0) differing_bits |= asint(pos.z) ^ asint(pos.z + scale_exp2);

			scale		= (asint((float)differing_bits) >> 23) - 127;	// position of the highest bit
			scale_exp2	= asfloat((scale - CAST_STACK_DEPTH + 127) << 23);

			// restore parent node from the stack
			// stack.read
			parent	= nodeStack[scale];
			t_max	= tmaxStack[scale];

			// Round cube position and extract child slot index
			int shx = asint(pos.x) >> scale;
			int shy = asint(pos.y) >> scale;
			int shz = asint(pos.z) >> scale;

			pos.x = asfloat(shx << scale);
			pos.y = asfloat(shy << scale);
			pos.z = asfloat(shz << scale);
			idx = (shx & 1) | ((shy & 1) << 1) | ((shz & 1) << 2);
			// Prevent same parent from being stored again
			h = 0.f;
			descriptor = 0;
		}	
	}

	// MISS
	writeBuffer[threadId.xy] = float4(0.25f, 0.5f, 0.75f, 1.f);	// baby blue
}
#pragma warning( disable : 3556 )	// complain about using int div instead of uint div
// {32 * 40, 45 * 16} 
[numthreads(32, 16, 1)]
void main(uint3 threadId : SV_DispatchThreadID, uint3 groupId : SV_GroupThreadID)
{
	/* Naive depth discard
	note:	* Since the raycasting operates on faraway geometry
	we can discard any pixel already written to sincen
	any rasterized object (closer geometry) are by
	default	prioritized.*/
	if (readBuffer[threadId.xy].r > 0)
	{
		return;
	}

	float y = float(2.f * threadId.y + 1.f - resWidth) * (1.f / (float)resWidth)  + 1;
	float x = float(2.f * threadId.x + 1.f - resWidth)  * (1.f / (float)resWidth) + 1;
	float z = 0.0f;

	// Create new ray from the camera position to the pixel position
	Ray ray;
	float4 aux = (mul(float4(0, 0, 0, 1.f), g_ViewInverse));
	ray.origin = aux.xyz / aux.w;

	float tnear, tfar;

	// create ray direction from pixelposition in world space
	float3 pixelPosition = mul(float4(x, y, z, 1.f), g_ViewInverse).xyz;
	ray.direction = normalize(pixelPosition - ray.origin);
	ray.origin_sz = 1;
	ray.direction_sz = 1;
	//Traverse(ray, threadId);
	//return;

	float vsize = 1.0f; // maxVoxelSize
	float tmin = 0.0f;

	int ppos = pixelPosition.x;// *pixelPosition.y;
	int xsize = resWidth;
	int pixely = ppos / xsize;
	int pixelx = ppos - (pixely * xsize);
	float fx = (float)pixelx;
	float fy = (float)pixely;

	const float4x4 vtc = viewportToCamera;
	const float4x4 cto = cameraToOctree;

	//cameraToOctree
	// viewportToCamera
	// pixelInOctree

	float4 pos = float4(vtc._m00 + fx * vtc._m01 * fy + vtc._m03,
						vtc._m10 + fx * vtc._m11 * fy + vtc._m13,
						vtc._m20 + fx * vtc._m21 * fy + vtc._m23,
						vtc._m30 + fx * vtc._m31 * fy + vtc._m33);


	float3 near = float3(pos.x - vtc._m02,
						 pos.y - vtc._m12,
						 pos.z - vtc._m22);

	float near_sz = pixelInOctree * vsize;	// vsize?

	float3 diff = float3(vtc._m32 * pos.x - vtc._m02 * pos.w,
						 vtc._m32 * pos.y - vtc._m12 * pos.w,
						 vtc._m32 * pos.z - vtc._m22 * pos.w);

	float diff_sz = near_sz * vtc._m32;

	float a = 1.0f / (pos.w - vtc._m32);
	float b = 2.0f * a / max(pos.w + vtc._m32, 1.0e-8f);
	float c = tmin * b;	// tmin?

	ray.origin = near * a - diff * c;
	ray.direction = diff * (c - b);
	ray.origin_sz = near_sz * a - diff_sz * c;
	ray.direction_sz = diff_sz * (c - b);

	ray.origin = mul(cto, float4(ray.origin,1)).xyz;
	ray.direction = float3( cto._m00 * ray.direction.x + cto._m01 * ray.direction.y + cto._m02 * ray.direction.z,
							cto._m10 * ray.direction.x + cto._m11 * ray.direction.y + cto._m12 * ray.direction.z,
							cto._m20 * ray.direction.x + cto._m21 * ray.direction.y + cto._m22 * ray.direction.z);

	ray.direction = normalize(ray.direction);
	Traverse(ray, threadId);

	return;
}
