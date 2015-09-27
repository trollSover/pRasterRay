#include "App.hlsl"

struct INPUT
{
	uint dataPtr	: DATA_PTR;
};

struct OUTPUT
{
	uint dataPtr	: DATA_PTR;
};

OUTPUT main(INPUT input)
{
	OUTPUT output;
	output.dataPtr = input.dataPtr;

	return output;
}