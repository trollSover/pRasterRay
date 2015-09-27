#pragma once

#include "../CoreSystems/CoreStd.h"

class TCamera
{
	/* Members */
private:
protected:
	/* Camera params */
	FVEC3	m_position,
			m_target,
			m_up;

	/* Frustum params */
	float	m_clientWidth,
			m_clientHeight,
			m_nearClip,
			m_farClip,
			m_fov;

	/* Matrices */
	FMAT4X4 m_view,
			m_proj,
			m_ortho,
			m_inverse,
			m_vp;
public:

	/* Methods */
private:
protected:
	virtual bool VInternalInit() { return false; }

public:
	TCamera();
	TCamera(const TCamera& _camera) { *this = _camera; }
	virtual ~TCamera() { }

	bool Init(const Resolution& _resolution, const float _near, const float _far, const float _fov);
	
	/*************
	--- Update ---
	**************/

	/* Camera */

	virtual void VMove(const FVEC3& _dir) { }
	virtual void VRotate(const FVEC3& _axis, float _degrees) { }
	

	/**************
	--- Setters ---
	***************/

	/* Camera */

	virtual void VSetPosition(const FVEC3& _pos)  { }
	virtual void VSetTarget(const FVEC3& _target) { }


	/**************
	--- Getters ---
	***************/

	/* Camera */

	FVEC3 GetPosition()		const { return m_position; }
	FVEC3 GetTarget()		const { return m_target; }
	FVEC3 LookAtTarget()	const { return FVEC3(m_up) - FVEC3(m_position); }	// why can't subtract operator be applied directly during const?
	FVEC3 GetUp()			const { return m_up; }

	/* Frustum */

	Resolution	GetClientResolution()	const { return Resolution(m_clientWidth, m_clientHeight); }
	float		GetFarthestPlane()		const { return m_farClip; }
	float		GetNearestPlane()		const { return m_farClip; }
	float		GetFov()				const { return m_fov; }
	
	/* Matrices */

	FMAT4X4 GetViewMatrix()			{ return m_view; }
	FMAT4X4 GetProjectionMatrix()	{ return m_proj; }
	FMAT4X4 GetOrthographicMatrix() { return m_ortho; }
	FMAT4X4 GetInverseVPMatrix()	{ return m_inverse; }
	FMAT4X4 GetVPMatrix()			{ return m_vp; }
};