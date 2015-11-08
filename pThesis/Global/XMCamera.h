#pragma once
#include "../CoreSystems/CoreStd.h"

class XMCamera
{
private:
	FVEC3	m_position;
	FVEC3	m_lookAt;
	FVEC3	m_up;
	FVEC3	m_forward;
	FVEC3	m_right;

	FMAT4X4	m_viewMatrix;
	FMAT4X4	m_projectionMatrix;


	float m_roll;
	float m_pitch;
	float m_yaw;

	int movementToggles[4];	//forward:back:left:right
	float movementSpeed;

protected:
public:
	FMAT4X4	m_rotationMatrix;
	FMAT4X4 m_inverseViewMatrix;
	FMAT4X4 m_inverseProjectionMatrix;
	FMAT4X4 m_inverseWVP;
	FMAT4X4	m_viewProjectionMatrix;

private:
	void UpdateView();
	void SetPerspectiveProjectionLH(float fov, float width, float height, float zNear, float zFar);

protected:
public:
	XMCamera();
	~XMCamera();

	void Update(const Time&);

	bool Init(unsigned int clientWidth, unsigned int clientHeight);
	void SetPositionAndView(float x, float y, float z, float hDeg, float pDeg);

	void SetMovementToggle(int i, int v);
	void AdjustHeadingPitch(float hRad, float pRad);

	void SetMovementSpeed(const float _unitsPerMS);
	float GetMovementSpeed(void) { return movementSpeed; }

	FMAT4X4 GetViewMatrix()			const { return m_viewMatrix; }
	FMAT4X4 GetProjectionMatrix()	const { return m_projectionMatrix; }
	FMAT4X4 GetInverseMatrix()		const { return m_inverseViewMatrix; }
	FMAT4X4 GetVPMatrix()			const { return m_viewProjectionMatrix; }
	FVEC3	GetPosition()			const { return m_position; }
	FVEC3	GetEyeDir()				const { return m_forward; }
	FVEC3	GetRight()				const { return m_right; }
	FVEC3	GetUp()					const { return m_up; }

	FMAT4X4 GetOrientation(void) const
	{
		FMAT4X4 r = TMatrixIdentity<float>();
		FVEC3 forward	= Normalize(m_forward);
		FVEC3 right		= Cross(m_up, forward);
		FVEC3 look		= Cross(forward, right);
		r.m[2][0] = -forward.x;
		r.m[2][1] = -forward.y;
		r.m[2][2] = -forward.z;
		r.m[0][0] = right.x;
		r.m[0][1] = right.y;
		r.m[0][2] = right.z;
		r.m[1][0] = look.x;
		r.m[1][1] = look.y;
		r.m[1][2] = look.z;
		return r;
	}
	FMAT4X4 WorldToCamera(void) const
	{
		FMAT4X4 r = m_rotationMatrix;
		r.m[0][3] = -m_position.x;
		r.m[1][3] = -m_position.y;
		r.m[2][3] = -m_position.z;
		r.m[3][3] = 1.f;
		return r;
	}
};