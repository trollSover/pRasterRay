#include "App.hlsl"

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	unsigned int index = DTid.y * 1024 + DTid.x;

	if (g_uRays[index].iTriangleId > (-2))
	{
		g_uIntersections[index] = IntersectP(g_uRays[index]);
	}
	else
	{
		g_uIntersections[index].iTriangleId = -2;
	}
}