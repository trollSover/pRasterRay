#include "TCamera.h"

using namespace VECTOR;
using namespace MATH;

TCamera::TCamera() :	
						m_clientHeight(0),
						m_clientWidth(0),
						m_nearClip(0),
						m_farClip(0),
						m_fov(0),	
						m_position(0.f, 0.f, -1.f),
						m_target(0.f, 0.f, 0.f),
						m_up(m_position + FVEC3(0.f, 1.f, 0.f))
{
	TMatrixIdentity(m_view);
	TMatrixIdentity(m_proj);
	TMatrixIdentity(m_ortho);

	m_vp		= m_view * m_proj;
	//m_inverse	= TMatrixInverse(m_vp);
}

bool TCamera::Init(const Resolution& _res, float _near, float _far, float _fov)
{
	m_clientWidth	= _res.width;
	m_clientHeight	= _res.height;
	m_nearClip		= _near;
	m_farClip		= _far;
	m_fov			= _fov;

	return VInternalInit();
}