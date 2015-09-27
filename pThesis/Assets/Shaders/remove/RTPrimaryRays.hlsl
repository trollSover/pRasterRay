#include "App.hlsl"

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint GIndex : SV_GroupIndex)
{
	const float N = 1024.f;

	float inverse = 1.0f / N;
	float y = -float(2.f * DTid.y + 1.f - N) * inverse;
	float x = float(2.f * DTid.x + 1.f - N) * inverse;
	float z = 2.0f;

	// Create new ray from the camera position to the pixel position
	Ray ray;
	float4 aux = (mul(float4(0, 0, 0, 1.f), g_mWorld));
	ray.vfOrigin = aux.xyz / aux.w;
	float3 vfPixelPosition = mul(float4(x, y, z, 1.f), g_mWorld).xyz;
	ray.vfDirection = normalize(vfPixelPosition - ray.vfOrigin);
	ray.fMaxT = 10000000000000000.f;
	ray.fMinT = 0;
	ray.vfReflectiveFactor = float3(1.f, 1.f, 1.f);
	ray.iTriangleId = -1;

	unsigned int index = DTid.y * N + DTid.x;
	// Copy ray to global UAV
	g_uRays[index] = ray;
	// Initialize accumulation buffer and result buffer
	g_uAccumulation[index] = 0.0f;
	g_uResultTexture[DTid.xy] = 0.f;
}