/******************************
--- Global Constant Buffers ---
*******************************/

cbuffer CBCamera : register(cb0)
{
	float4x4 g_WorldViewProjection;			// 64
	float4x4 g_View;						// 64
	float4x4 g_Projection;					// 64
	float4x4 g_World;						// 64

	float4x4 g_ViewInverse;					// 64
	float4x4 g_ProjectionInverse;			// 64
	float4x4 g_WorldViewProjectionInverse;	// 64
	float4x4 g_Rotation;					// 64

	float3	 g_cameraPos;					// 12
	float3	 g_cameraDir;					// 12
	float3	 g_right;						// 12
	float3	 g_up;							// 12
};											// = 544 bytes (16 * 34)

cbuffer CBWindow : register(cb1)
{
	uint  g_resolutionWidth;	// 4
	uint  g_resolutionHeight;	// 4
	uint2 padding2;				// 8
};								// = 16 bytes (16 * 1)

#define Rad90 90.f * 3.1415 / 180.f
#define CosRad90 cos(Rad90)
#define SinRad90 sin(Rad90)

cbuffer CBRotatePlane
{
	static const float4x4 rotateAxisX = float4x4(1, 0, 0, 0,
									0, CosRad90, -SinRad90, 0,
									0, SinRad90, CosRad90, 0,
									0, 0, 0, 1);
	static const float4x4 rotateAxisY = float4x4(CosRad90, 0, SinRad90, 0,
									0, 1, 0, 0,
									-SinRad90, 0, CosRad90, 0,
									0, 0, 0, 1);
	static float4x4 rotateAxisZ = float4x4(CosRad90, -SinRad90, 0, 0,
									SinRad90, CosRad90, 0, 0,
									0, 0, 1, 0,
									0, 0, 0, 1);
}