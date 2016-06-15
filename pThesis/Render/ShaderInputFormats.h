#pragma once

#include "../Utils/Vector_Util.h"

using namespace VECTOR;

struct Vertex
{
	VEC3<float> p;
	VEC3<float> n;
	VEC4<float> c;
		

	Vertex(VEC3<float> _p = VEC3<float>(), VEC3<float> _n = VEC3<float>(), VEC4<float> _c = VEC4<float>())
		: p(_p), n(_n), c(_c)
	{		}
};

struct BBPoint
{
	FVEC4 p;

	BBPoint(FVEC4 _p = FVEC4())
		: p(_p)
	{	}
};

static const FVEC3 BoxCorners[8] =
{
	FVEC3(0, 0, 0),	// front bottom left
	FVEC3(1, 0, 0),	// front bottom right
	FVEC3(0, 1, 0),	// front top left
	FVEC3(1, 1, 0),	// front top right
	FVEC3(0, 0, 1),	// back bottom left
	FVEC3(1, 0, 1),	// back bottom right
	FVEC3(0, 1, 1),	// back top left
	FVEC3(1, 1, 1)	// back top right
};

static const int BoxIndices[] =
{
	0, 2, 1, 2, 3, 1,	// front
	4, 5, 6, 5, 7, 6,	// back
	4, 0, 1, 1, 5, 4,	// bottom
	6, 3, 2, 6, 7, 3,	// top
	0, 4, 6, 6, 2, 0,	// left
	7, 5, 1, 1, 3, 7	// right
};

struct BoundingVolumePerObject
{
	FVEC3 transformation;
	float size;
	FVEC4 color;
};