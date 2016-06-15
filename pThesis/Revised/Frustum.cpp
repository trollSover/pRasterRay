#include "Frustum.h"

using RasterRay::Frustum;

Frustum::Frustum(void) : m_wvpMatrix(), m_viewMatrix(), m_sceneCB({ 0 }), m_svoHeader()
{
	for (unsigned i = 0; i < 6; ++i)
		m_planes[i] = { 0 };
}

Frustum::~Frustum(void)
{	}

void Frustum::Build(const float _depth, FMAT4X4 _projectionMatrix, FMAT4X4 _viewMatrix)
{
	float zMin, r;
	FMAT4X4 matrix;
	m_wvpMatrix		= TMatrixIdentity<float>() * _viewMatrix * _projectionMatrix;
	m_viewMatrix	= TMatrixIdentity<float>() * _viewMatrix;
	zMin = -_projectionMatrix.m[3][2] / _projectionMatrix.m[2][2];		// change for different starting depths
	r = _depth / (_depth - zMin);
	_projectionMatrix.m[2][2] = r;
	_projectionMatrix.m[3][2] = -r * zMin;

	matrix = _viewMatrix * _projectionMatrix;
	//VPMatrix = matrix;
	// near plane
	m_planes[0].x = matrix.m[0][3] + matrix.m[0][2];
	m_planes[0].y = matrix.m[1][3] + matrix.m[1][2];
	m_planes[0].z = matrix.m[2][3] + matrix.m[2][2];
	m_planes[0].w = matrix.m[3][3] + matrix.m[3][2];
	m_planes[0] = Normalize(m_planes[0]);

	// far plane
	m_planes[1].x = matrix.m[0][3] - matrix.m[0][2];
	m_planes[1].y = matrix.m[1][3] - matrix.m[1][2];
	m_planes[1].z = matrix.m[2][3] - matrix.m[2][2];
	m_planes[1].w = matrix.m[3][3] - matrix.m[3][2];
	m_planes[1] = Normalize(m_planes[1]);

	// left plane
	m_planes[2].x = matrix.m[0][3] + matrix.m[0][0];
	m_planes[2].y = matrix.m[1][3] + matrix.m[1][0];
	m_planes[2].z = matrix.m[2][3] + matrix.m[2][0];
	m_planes[2].w = matrix.m[3][3] + matrix.m[3][0];
	m_planes[2] = Normalize(m_planes[2]);

	// right plane
	m_planes[3].x = matrix.m[0][3] - matrix.m[0][0];
	m_planes[3].y = matrix.m[1][3] - matrix.m[1][0];
	m_planes[3].z = matrix.m[2][3] - matrix.m[2][0];
	m_planes[3].w = matrix.m[3][3] - matrix.m[3][0];
	m_planes[3] = Normalize(m_planes[3]);

	// top
	m_planes[4].x = matrix.m[0][3] - matrix.m[0][1];
	m_planes[4].y = matrix.m[1][3] - matrix.m[1][1];
	m_planes[4].z = matrix.m[2][3] - matrix.m[2][1];
	m_planes[4].w = matrix.m[3][3] - matrix.m[3][1];
	m_planes[4] = Normalize(m_planes[4]);

	// bottom
	m_planes[5].x = matrix.m[0][3] + matrix.m[0][1];
	m_planes[5].y = matrix.m[1][3] + matrix.m[1][1];
	m_planes[5].z = matrix.m[2][3] + matrix.m[2][1];
	m_planes[5].w = matrix.m[3][3] + matrix.m[3][1];
	m_planes[5] = Normalize(m_planes[5]);


	/* order of planes:
	0: near
	1: far
	2: left
	3: right
	4: top
	5: bottom
	*/
}