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

	static const float3 wmin = mul(float4(0, 0, 0, 1), g_ViewInverse).xyz;
	static const float3 wmax = mul(float4(gridLength, gridLength, gridLength, 1), g_ViewInverse).xyz;
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
	return  0 == (Nodes[_nodePtr].children & ((1 << 8) - 1));

	//return  0 == (Nodes[_nodePtr].children & ((1 << 8) - 1)) && Nodes[_nodePtr].dataPtr != 0;
}

bool IsNull(const in uint _nodePtr)
{
	return IsLeaf(_nodePtr) && (Nodes[_nodePtr].dataPtr == 0);
}


#pragma warning( disable : 4714 )	// due to CAST_STACK_DEPTH we reach the limit of temp registers, will negatively affect performance - disable warning for now

#define CAST_STACK_DEPTH 23
#define MAX_ITERATIONS  100
static const float ITR_DEPTH = 1.f / (float)MAX_ITERATIONS;

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

	const float epsilon						= exp2(-CAST_STACK_DEPTH);
	const int	INVALID_CHILD_DESCRIPTOR	= 0;	// no bits set

	// avoid division by zero on small ray direction
	//if (abs(_ray.direction.x) < epsilon) _ray.direction.x = copysign(epsilon, _ray.direction.x); //(epsilon * sign(_ray.direction.x));
	//if (abs(_ray.direction.y) < epsilon) _ray.direction.y = copysign(epsilon, _ray.direction.y); //(epsilon * sign(_ray.direction.y));
	//if (abs(_ray.direction.z) < epsilon) _ray.direction.z = copysign(epsilon, _ray.direction.z); //(epsilon * sign(_ray.direction.z));

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
	float t_max = max(max(2.f * tx_cf - tx_bs, 2.f * ty_cf - ty_bs), 2.f * tz_cf - tz_bs);
	float t_min = min(min(tx_cf - tx_bs, ty_cf - ty_bs), tz_cf - tz_bs);
	float h		= t_max;
	t_min		= max(t_min, 0.f);
	t_max		= min(t_max, 1.f);

	// init current node
	int		parent		= rootIndex;					// current root ptr
	int		descriptor	= INVALID_CHILD_DESCRIPTOR;
	int		idx			= 0;							// child slot
	float3	pos			= float3(1.f, 1.f, 1.f);
	int		scale		= CAST_STACK_DEPTH - 1;
	float	scale_exp2	= 0.5f;							// volume divider

	if (1.5f * tx_cf - tx_bs > t_min) idx ^= 1, pos.x = 1.5f;
	if (1.5f * ty_cf - ty_bs > t_min) idx ^= 2, pos.y = 1.5f;
	if (1.5f * tz_cf - tz_bs > t_min) idx ^= 4, pos.z = 1.5f;
	
	int itr = 0;

	float ray_dir_sz =_ray.direction_sz;
	float ray_pos_sz =_ray.origin_sz;

	float cx = (float)(threadId.x) / (float)resWidth;
	float cy = (float)(threadId.y) / (float)resHeight;

	float4 color = BLACK;// Voxels[parent].color;

	// traverse nodes along ray within the octree
	while (scale < CAST_STACK_DEPTH)
	{
		itr++;
		if (itr > MAX_ITERATIONS)	// iteration ceiling reached - abort
			break;

		if (idx < 0 || idx > 7 || parent < 0 || parent > (int)rootIndex)	// debug sanity check
		{
			writeBuffer[threadId.xy] = YELLOW;
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
		float tc_max	= min(min(tx_corner, ty_corner), tz_corner);

		int child_shift = idx ^ octant_mask;
		int child_masks = descriptor << child_shift;

		// if ray hits root this will always be true on the first iteration
		//if ((child_masks & 0x8000) != 0 && t_min <= t_max)
		if (t_min <= t_max)
		{
			//color = RED;
			//color = colors[4];	// PURPLE
			//break;

			if ((tc_max * ray_dir_sz + ray_pos_sz) >= scale_exp2)
			{
				int node		= GetChildAddress(parent, idx);
				int voxelptr	= Nodes[node].dataPtr;
				color = Voxels[voxelptr].color;
				writeBuffer[threadId.xy] = color;
				//writeBuffer[threadId.xy] = WHITE;
				return;
				break;
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
				color = GREEN;
				if ((child_masks & 0x0080) == 0)
				{
					int node		= GetChildAddress(parent, idx);
					int voxelptr	= Nodes[node].dataPtr;
					color = Voxels[voxelptr].color;
					writeBuffer[threadId.xy] = color;
					//writeBuffer[threadId.xy] = WHITE;
					return;
					break; // at t_min (overridden with tv_min).
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
				int offset = (unsigned int)descriptor >> 24;
				//if ((descriptor & 0x10000) != 0)	// far - not our concern
				//	break;

				//offset += popc8(child_masks & 0x7F);

				const int val = offset * 2;
				if (val < 0 || val > 7)
				{
					writeBuffer[threadId.xy] = PURPLE;
					//return;
				}

				if (HasChildAtIndex(parent, val))
				{
					parent = GetChildAddress(parent, val);
				}
				else
				{
					parent -= val;
					writeBuffer[threadId.xy] = RED;
					//return;
				}
				parent = rootIndex;
				//parent += offset * 2;
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
			//color = YELLOW;
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

	//float3 normal = cross(pos - _ray.direction, pos - _ray.origin);
	//	normal = normalize(normal);
	//writeBuffer[threadId.xy] = float4(normal, 1);

	//if (IsLeaf(parent))
	//	writeBuffer[threadId.xy] = color;
	//else
	//{
	//	float c  = itr * ITR_DEPTH;
	//	float4 col = float4(c, c, c, 1);
	//	writeBuffer[threadId.xy] = col;
	//}
}

void ATraverse(inout Ray _ray, uint2 threadId)
{
	/* iteration variables */
	int node  = rootIndex;
	int depth = 0;
	float scale = 1;
	const float dim = 1 << rootDepth;// pow(2, rootDepth - 1);// / (float)resWidth;
	float tmin, tmax;
	

	//float3 dmin = float3(0, 0, 0);
	//float3 dmax = float3(dim, dim, dim);
	float3 dmin = float3(dim,dim,dim) * -0.5;
	float3 dmax = float3(dim, dim, dim) * 0.5;

	struct Stack
	{
		int		nodeptr;
		float	scale;
		float   smin, smax;
		float3  svmin, svmax;
	};

	if (!Intersect(_ray, dmin, dmax, tmin, tmax))
	{
		return;
	}


	Stack stack[25];
	stack[0].nodeptr = rootIndex;
	stack[0].scale = 1.f;
	stack[0].smin = tmin;
	stack[0].smax = tmax;
	stack[0].svmin = dmin;
	stack[0].svmax = dmax;

	int itr = 0;
	const int ITR_MAX = 10000;

	while (depth < (int)rootDepth)
	{
		if (itr > ITR_MAX)
			break;

		itr++;

		if (depth < 0)
			break;

		if (IsLeaf(stack[depth].nodeptr))
		{
			//writeBuffer[threadId.xy] = RED;
			break;
		}

		scale = stack[depth].scale;
		tmin  = stack[depth].smin;
		tmax  = stack[depth].smax;
		dmin  = stack[depth].svmin;
		dmax  = stack[depth].svmax;

		// find the closest child
		if (Intersect(_ray, dmin, dmax, tmin, tmax))
		{
			for (int i = 0; i < 8; ++i)
			{
				if (HasChildAtIndex(stack[depth].nodeptr, i))
				{
					float3 dvmin = dmin + pos[i] * (dim * scale * 0.5);		// parent min + child offset
					float3 dvmax = dvmin + pos[7] * (dim * scale * 0.5);	// child min + child dim

					float tvmin, tvmax;

					if (Intersect(_ray, dvmin, dvmax, tvmin, tvmax))
					{
						
						if (tvmin < tmax)
						{
							//tmax = tvmin;
							int child = GetChildAddress(stack[depth].nodeptr, i);

							//int 
							stack[depth + 1].nodeptr = child;
							stack[depth + 1].scale = scale * 0.5f;
							stack[depth + 1].smin = tvmin;
							stack[depth + 1].smax = tvmax;
							stack[depth + 1].svmin = dvmin;
							stack[depth + 1].svmax = dvmax;

							stack[depth + 1].smin = max(tvmin, tmin);
							stack[depth + 1].smax = min(tvmax, tmax);
							//if (IsLeaf(child))
							{
								int voxelptr = Nodes[child].dataPtr;
								float3 nor = Voxels[voxelptr].normal;
								float4 col = Voxels[voxelptr].color / 256.f;
								col.w = 1;

								static const float4 ambientcolor	= float4(0.5f, 0.5f, 0.5f, 1.0f);
								static const float4 diffuseColor	= float4(1.0f, 1.0f, 1.0f, 1.0f);
								static const float3 lightDirection	= float3(0.0f, -1.0f, 0.5f);

								float4 color = col * ambientcolor;
								float	lightIntensity = saturate(dot(nor, lightDirection));
								if (lightIntensity > 0.f)
									color += (diffuseColor * lightIntensity);

								if (voxelptr == 0)
								{
									writeBuffer[threadId.xy] = PURPLE;
								}
								else
								{
									writeBuffer[threadId.xy] = color;
								}
									
								//	float c = (float)depth / (float)rootDepth;
								//float4 color = colors[i];
								//	color.w = 1;
								//writeBuffer[threadId.xy] = float4(c,c,c,1);
								//writeBuffer[threadId.xy] = colors[i];
								//break;
							}
							//else
							{
								float c = (float)depth / (float)rootDepth;
								//continue;
							}
						}

					}
				}
			}

			depth++;
			continue;
		}
		//else
		{
			stack[depth - 1] = stack[depth];
			node = stack[depth].nodeptr;
			depth--;
		}
	}



	return;
}

void BTraverse(in Ray _ray, in uint2 pixel)
{
	const int dim = gridLength;

	// stack RAM usage(B) = sizeof(Stack) * resWidth * resHeight
	struct Stack
	{
		int		nodeptr;
		int		idx;
		float	scale;
		float3	origin;
	};

	int stackIndex = STACK_LIMIT - 1;
	Stack stack[STACK_LIMIT];

	stack[stackIndex].nodeptr = rootIndex;
	stack[stackIndex].idx = 0;
	stack[stackIndex].scale = 1;
	stack[stackIndex].origin = float3(dim, dim, dim) * -0.5f;

	int itr = 0;

	writeBuffer[pixel.xy] = float4(244.f / 256.f, 164.f / 256.f, 96.f / 256.f, 1);
	float lim = 1000000;
	while (stackIndex < STACK_LIMIT)
	{
		itr++;

		if (stackIndex < 0)
		{
			//writeBuffer[pixel.xy] = RED;
			break;
		}

		if (itr > ITR_LIMIT)
		{
			//writeBuffer[pixel.xy] = YELLOW;
			break;
		}

		int		node	= stack[stackIndex].nodeptr;
		int		idx		= stack[stackIndex].idx;
		float	scale	= stack[stackIndex].scale;
		float3	origin	= stack[stackIndex].origin;

		// does the current node intersect the ray?
		float3 omin = origin;
		float3 omax = origin + pos[7] * (dim * scale);
		float  tmin, tmax;


		if (idx < 7)
		{
			stack[stackIndex].idx++;	// advance in advance
		}
		else
		{
			stackIndex++;
			continue;
		}

		if (IsNull(node))
		{
			stackIndex++;
			continue;
		}

		if (IsLeaf(node) && Nodes[node].dataPtr != 0)
		{
			int voxelptr = Nodes[node].dataPtr;
			float4 color = Voxels[voxelptr].color / 256.f;
				color.w = 1;
			writeBuffer[pixel.xy] = color;
			return;
		}

		float3 tomin = omin + pos[idx] * (dim * scale * 0.5f);
		float3 tomax = tomin + pos[7] * (dim * scale * 0.5f);
		float tvmin, tvmax;


		if (Intersect(_ray, tomin, tomax, tvmin, tvmax))
		{
			//if (tvmin > 0)
			//	continue;

			//if (min(tvmin, tvmax) < lim)
			{
				if (HasChildAtIndex(node, idx))
				{
					lim = max(tvmin, tvmax);
					//writeBuffer[pixel.xy] = colors[idx];

					// child intersect - ADD
					stackIndex--;
					stack[stackIndex].nodeptr = GetChildAddress(node, idx);
					stack[stackIndex].idx = 0;
					stack[stackIndex].scale = scale * 0.5f;
					stack[stackIndex].origin = tomin;
				}
			}
		}

		

		continue;

		// do node intersect ?
		if (Intersect(_ray, omin, omax, tmin, tmax) && true == false)
		{
			// node is leaf but not necessarily a terminating node (LoD dependent)
			if (IsLeaf(node))
			{
				if (scale * dim > 1.f)
					continue;

				int voxelptr = Nodes[node].dataPtr;		
				float4 color = Voxels[voxelptr].color / 256.f;
				color.w = 1;
				writeBuffer[pixel.xy] = color;
				//return;		
				stackIndex--;
				return;
			}
			else if (HasChildAtIndex(node, idx))
			{
				float ci = (float)itr / (float)ITR_LIMIT;
				float cs = (float)stackIndex / (float)STACK_LIMIT;
				//writeBuffer[pixel.xy] = float4(c, c, c, 1);
				//writeBuffer[pixel.xy] = colors[idx] * c;
				writeBuffer[pixel.xy] = colors[idx] * cs;
				int tidx = idx;
				// ADVANCE TO CHILD
				if (idx < 7)
					stack[stackIndex].idx = idx + 1;
				float3 tomin = omin  + pos[idx] * (dim * scale * 0.5f);
				float3 tomax = tomin + pos[7] * (dim * scale * 0.5f);
				float tvmin, tvmax;

				// do node child[idx] intersect?
				if (Intersect(_ray, tomin, tomax, tvmin, tvmax))
				{
					//writeBuffer[pixel.xy] = colors[tidx];

					

					// child intersect - ADD
					stackIndex--;
					stack[stackIndex].nodeptr	= GetChildAddress(node, idx);
					stack[stackIndex].idx		= 0;
					stack[stackIndex].scale		= scale * 0.5f;
					stack[stackIndex].origin	= tomin;

					continue;
				}
			}

			if (idx < 7)
			{
				stack[stackIndex].idx = idx + 1;
				continue;
			}
		}

		// no intersection - POP

		stackIndex++;
		//stack[stackIndex].idx = stack[stackIndex].idx + 1;	// advance idx
	}


	float sc = (float)stackIndex / (float)STACK_LIMIT;
	//writeBuffer[pixel.xy] = float4(sc, sc, sc, 1);
	return;
	if (stackIndex < 0)
	{
		//writeBuffer[pixel.xy] = YELLOW;
		return;
	}
	else if (stackIndex >= STACK_LIMIT)
	{
		
		float c = (float)itr / (float)ITR_LIMIT;
		//writeBuffer[pixel.xy] = float4(c,c,c,1);
		return;
	}

	float c = (float)stackIndex / (float)STACK_LIMIT;
	float4 color = colors[stack[stackIndex].idx];
	//writeBuffer[pixel.xy] = float4(255.f / 256.f, 105.f / 256.f, 180.f / 256.f, 1);// GREENBLUE; float4(c, c, c, 1);
}







//[numthreads(THREAD_COUNT_X, THREAD_COUNT_Y, 1)]
//void main(uint3 threadId : SV_DispatchThreadID, uint3 groupId : SV_GroupThreadID, uint groupIndex : SV_GroupIndex)
//{
//	unsigned int2 tId = threadId.xy;
//	tId.x = WORK_SIZE_X * groupId.x;
//	tId.y = WORK_SIZE_Y * groupId.y;
//	unsigned int id = groupIndex; // groupId.x + groupId.y;
//
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
//			ATraverse(ray, tId, threadId.xy, id % 8);
//		}
//	}
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


#pragma warning( disable : 3556 )	// complain about using int div instead of uint div
 //{32 * 40, 45 * 16} 
[numthreads(32, 16, 1)]
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

	float y = float(2.f * threadId.y + 1.f - resHeight) * (1.f / (float)resHeight) + 1;
	float x = float(2.f * threadId.x + 1.f - resWidth)  * (1.f / (float)resWidth) + 1;
	//float y = (float)threadId.y * (1.f / (resHeight * 0.5f)) + 1.f;
	//float x = (float)threadId.x * (1.f / (resWidth  * 0.5f)) - 1.f;
	float z = 1.0f;

	// Create new ray from the camera position to the pixel position
	Ray ray;
	float4 aux = (mul(float4(0, 0, 0, 1.f), g_ViewInverse));
	ray.origin = aux.xyz / aux.w;
	ray.origin_sz = 0;
	ray.direction_sz = 0;

	float tnear, tfar;

	// create ray direction from pixelposition in world space
	float3 pixelPosition = mul(float4(x, y, z, 1.f), g_ViewInverse).xyz;
	ray.direction = normalize(pixelPosition - ray.origin);
	ray.origin_sz = 0;
	ray.direction_sz = 0;

	ray.origin = g_cameraPos;

	BTraverse(ray, threadId.xy);
	return;

	
	int		xsize	= resWidth;
	float	vsize	= 1.0f; // maxVoxelSize
	float	tmin	= 0.0f;


	// 1:1 mapping thread:pixel 
	float fx = threadId.x;
	float fy = threadId.y;

	const float4x4 vtc = viewportToCamera;
	const float4x4 cto = cameraToOctree;

	//cameraToOctree
	// viewportToCamera
	// pixelInOctree

	float4 pos = float4(vtc._m00 + fx * vtc._m01 * fy + vtc._m03,
						vtc._m10 + fx * vtc._m11 * fy + vtc._m13,
						vtc._m20 + fx * vtc._m21 * fy + vtc._m23,
						vtc._m30 + fx * vtc._m31 * fy + vtc._m33);

	//float4 pos = mul(float4(fx, fy, 1, 1), vtc);

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

	ray.direction = mul(float4(ray.direction,1), cto).xyz;
	ray.direction = normalize(ray.direction);

	ATraverse(ray, threadId.xy);
	
	return;
}
