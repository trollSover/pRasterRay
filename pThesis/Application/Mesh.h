#pragma once

#include "STD.h"
#include "Vertex.h"

class Mesh
{
	/* Members */
private:
protected:
	AABB<float> m_box;
public:

	/* Methods */
private:
protected:
public:
	virtual ~Mesh(void) {}
	Mesh(void) {}

	virtual INDEX	 VGetNextIndices(void)	= 0;
	virtual TRIANGLE VGetNextTriangle(void) = 0;
	virtual VERTEX	 VGetNextVertex(void)	= 0;
	virtual bool	 VEndOf(void)			= 0;

	AABB<float>		 GetAABB(void)	const { return m_box; }

	virtual TRIANGLE VGetTriangle(const INDEX& _index) = 0;

	virtual uint32_t VGetCurrentIndex(void) = 0;
	virtual uint32_t VGetNElements(void)	= 0;
};

