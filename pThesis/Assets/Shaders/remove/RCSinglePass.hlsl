#include "App.hlsl"

struct CRay
{
	float3 origin;
	float3 dir;
};

struct AABB
{
	float3 min;
	float3 max;
};

static const float Absorb		= 1.0f;
static const float maxDist		= sqrt(2.0);
static const int   numSamples	= 128;


bool IntersectBox(CRay r, AABB aabb, out float t0, out float t1)
{
	float3 invR		= 1.0f / r.dir;
	float3 tbottom	= invR * (aabb.min - r.origin);
	float3 ttop		= invR * (aabb.max - r.origin);
	float3 tmin		= min(ttop, tbottom);
	float3 tmax		= max(ttop, tbottom);
	float2 t		= max(tmin.xx, tmax.yz);

	t0	= max(t.x, t.y);
	t	= min(tmax.xx, tmax.yz);
	t1	= min(t.x, t.y);

	return t0 <= t1;
}


[numthreads(32, 32, 1)]
void main(uint3 threadId : SV_DispatchThreadID, uint3 groudId : SV_GroupThreadID)
{
	const float4 dim = float4(10000, 10000, 10000, 0);

	float4 position = mul(float4(0, 0, 0, 1), g_mWVP);
	float4 min = mul(position - dim, g_mWVP);
	float4 max = mul(position + dim, g_mWVP);
	
	/* outside clipspace */
	if (position.x < -1.f || position.x > 1.f || position.y < -1.f || position.y > 1.f || position.z < -1.f || position.z > 1.f)
		return;

	for (int i = min.x; i < max.x; ++i)
	{
		for (int j = min.y; j < max.y; ++j)
		{
			int2 coord = int2(i, j);
				g_uResultTexture[coord + threadId.xy] = float4(1, 0, 0, 1);
		}
	}
	g_uResultTexture[threadId.xy] = float4(1, 0, 0, 1);
}
