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