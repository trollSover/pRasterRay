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
	float	m_focalLength;

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
	FMAT4X4 worldToCamera;

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
	void AdjustYaw(float _rad);
	void AdjustRoll(float _rad);
	void AdjustPitch(float _rad);

	void SetMovementSpeed(const float _unitsPerMS);
	float GetMovementSpeed(void) { return movementSpeed; }

	FMAT4X4 GetViewMatrix()			const { return m_viewMatrix; }
	FMAT4X4 GetProjectionMatrix()	const { return m_projectionMatrix; }
	FMAT4X4 GetInverseMatrix()		const { return m_inverseViewMatrix; }
	FMAT4X4 GetVPMatrix()			const { return m_viewProjectionMatrix; }

	FMAT4X4 GetViewInverse			(void)	const { return TMatrixInverse(m_viewMatrix); }
	FMAT4X4 GetProjectionInverse	(void)	const { return TMatrixInverse(m_projectionMatrix); }
	FMAT4X4 GetViewProjectionInverse(void)	const { return TMatrixInverse(m_viewProjectionMatrix); }

	FVEC3	GetPosition()			const { return m_position; }
	FVEC3	GetEyeDir()				const { return m_forward; }
	FVEC3	GetRight()				const { return m_right; }
	FVEC3	GetUp()					const { return m_up; }
	float	GetFocalLength()		const { return m_focalLength; }

	FMAT4X4 WorldToCamera(void) const
	{
		return m_viewMatrix;
	}
};