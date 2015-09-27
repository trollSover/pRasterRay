#include "App.hlsl"
//#include "MarchingCubes.hlsl"

struct INPUT
{
	uint dataPtr	: DATA_PTR;
};

struct OUTPUT
{
	float4 position : SV_POSITION;
};

static const int MCOrder[8] = { 0, 1, 4, 5, 2, 3, 6, 7 };

[maxvertexcount(15)]
void main(lineadj INPUT input[1], inout TriangleStream<OUTPUT> triStream)
{
	OUTPUT output;


	output.position = float4(1, 1, 1, 1);
	triStream.Append(output);
	//for (int i = 0; i < 8; ++i)
	//{
	//	cell.p[i] = float3(0, 0, 0);
	//	cell.val[i] = 0;
	//}

	//cell.p[0] = MortonDecode(Voxels[input[0].dataPtr].morton);
	////cell.p[1] = MortonDecode(Voxels[input[1].dataPtr].morton);
	////cell.p[4] = MortonDecode(Voxels[input[2].dataPtr].morton);
	////cell.p[5] = MortonDecode(Voxels[input[3].dataPtr].morton);
	//cell.val[0] = cell.val[1] = cell.val[4] = cell.val[5] = 1;
	//TRIANGLE tris[5];
	//int N = Polygonize(cell, 0.5, tris);
	//for (int i = 0; i < 1; ++i)
	//{
	//	output.position = mul(float4(tris[i].p[0], 1), g_mWVP);
	//	triStream.Append(output);
	//	//output.position = mul(float4(tris[i].p[1], 1), g_mWVP);
	//	//.Append(output);
	//	//output.position = mul(float4(tris[i].p[2], 1), g_mWVP);
	//	//triStream.Append(output);
	//}
}