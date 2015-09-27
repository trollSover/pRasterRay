//--------------------------------------------------------------------------------------------------------------------
// COLOR STAGE
//--------------------------------------------------------------------------------------------------------------------

#include "App.hlsl"

float2 computeUVs(int iTriangleId, Intersection intersection)
{
	int offset = iTriangleId *1;
	float2 A, B, C;

	//uint dataPtr = Nodes[g_sIndices[offset]].dataPtr;
	uint dataPtr = g_sIndices[offset];

	//A.xy = Voxels[g_sIndices[offset]].data.uv;
	//B.xy = Voxels[g_sIndices[offset + 1]].data.uv;
	//C.xy = Voxels[g_sIndices[offset + 2]].data.uv;

	A.xy = float2(0,0);//Voxels[dataPtr].data.uv;
	B.xy = float2(0,0);//Voxels[dataPtr + 1].data.uv;
	C.xy = float2(0,0);//Voxels[dataPtr + 2].data.uv;

	return intersection.fU* B.xy + intersection.fV * C.xy + (1.0f - (intersection.fU + intersection.fV))*A;
}

float3 Vec3BaryCentric(float3 V1, float3 V2, float3 V3, float2 UV)
{
	float3 fOut = 0.f;
	float t = 1.f - (UV.x + UV.y);

	fOut[0] = V1[0] * t + V2[0] * UV.x + V3[0] * UV.y;
	fOut[1] = V1[1] * t + V2[1] * UV.x + V3[1] * UV.y;
	fOut[2] = V1[2] * t + V2[2] * UV.x + V3[2] * UV.y;

	return fOut;
}

static const float3 TILEX[2] = { float3(0,0,0), float3(1,0,0) };
static const float3 TILEY[2] = { float3(0, 1, 0), float3(0, 0, 1) };

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GId : SV_GroupID)
{
	const int N = resWidth;

	// Calculate 1D index of the current thread
	const unsigned int index = DTid.y * N + DTid.x;

	// Get the triangle ID stored in the current pixel
	const int iTriangleId = g_uIntersections[index].iTriangleId;

	// If Environment Mapping has already been applied to this ray, return.
	int g_iEnvMappingFlag = -2;
	if (iTriangleId < g_iEnvMappingFlag)
	{
		if (iTriangleId >(-2))
			g_uResultTexture[DTid.xy] = 1.f;

		return;
	}

	// Init variables
	float4 vfFinalColor = float4(0.f, 0.f, 0.f, 1.f);

	if (iTriangleId >= 0)
	{
		vfFinalColor = float4(0, 1, 0, 1);
		//float2 vfUV = computeUVs(iTriangleId, g_uIntersections[index]);
		g_uRays[index].iTriangleId = iTriangleId;
	}
	else
	{
		vfFinalColor = float4(1, 0, 0, 0);
		g_uRays[index].iTriangleId = -2;

	}

	// Apply color to texture
	g_uAccumulation[index]	  += vfFinalColor;
	g_uResultTexture[DTid.xy] = g_uAccumulation[index];
}