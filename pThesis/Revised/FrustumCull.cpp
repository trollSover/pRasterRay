#include "Frustum.h"

using RasterRay::Frustum;

unsigned Frustum::EnclosedNode(const float _x, const float _y, const float _z, const float _dim)
{
	// http://www.racer.nl/reference/vfc_markmorley.htm

	int c;
	int c2 = 0;

	/* order of planes:
	0: near
	1: far
	2: left
	3: right
	4: top
	5: bottom
	*/

	// PlaneDotCoord = (_v.x * _plane.x + _v.y * _plane.y + _v.z * _plane.z + _plane.w * 1);
	for (unsigned i = 0; i < 6; ++i)
	{
		float k = PlaneDotCoord(FVEC3(_x, _y, _z), m_planes[i]);
		float kk = m_planes[0].x * _x + m_planes[0].y * _y + m_planes[0].z * _z + m_planes[0].w;
		bool b = k == kk;
		c = 0;
		if (PlaneDotCoord(FVEC3(_x, _y, _z), m_planes[i]) > 0)
			c++;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y, _z), m_planes[i]) > 0)
			c++;
		if (PlaneDotCoord(FVEC3(_x, _y + _dim, _z), m_planes[i]) > 0)
			c++;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y + _dim, _z), m_planes[i]) > 0)
			c++;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y, _z + _dim), m_planes[i]) > 0)
			c++;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y, _z + _dim), m_planes[i]) > 0)
			c++;
		if (PlaneDotCoord(FVEC3(_x, _y + _dim, _z + _dim), m_planes[i]) > 0)
			c++;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y + _dim, _z + _dim), m_planes[i]) > 0)
			c++;
		if (c == 0)
			return 0;
		if (c == 8)
		{
			c2++;
		}
	}

	return (c2 == 6) ? 2 : 1;
}

bool Frustum::IntersectNode(const float _x, const float _y, const float _z, const float _dim)
{
	for (int i = 0; i < 6; ++i)
	{
		if (PlaneDotCoord(FVEC3(_x - _dim, _y - _dim, _z - _dim), m_planes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y - _dim, _z - _dim), m_planes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x - _dim, _y + _dim, _z - _dim), m_planes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y + _dim, _z - _dim), m_planes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x - _dim, _y - _dim, _z + _dim), m_planes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y - _dim, _z + _dim), m_planes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x - _dim, _y + _dim, _z + _dim), m_planes[i]) >= 0)
			continue;
		if (PlaneDotCoord(FVEC3(_x + _dim, _y + _dim, _z + _dim), m_planes[i]) >= 0)
			continue;
		return false;
	}
	return true;
}