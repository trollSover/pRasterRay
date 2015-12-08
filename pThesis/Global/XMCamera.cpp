#include "XMCamera.h"

#define TWO_PI		6.283185307179586476925286766559
#define DEG_TO_RAD	0.01745329251994329576923690768489


XMCamera::XMCamera() :	m_lookAt(0, 0, 0),
						m_position(0, 0, -1),
						m_up(0, 1, 0),
						m_roll(0),
						m_pitch(0),
						m_yaw(0),
						m_forward(0, 0, 1),
						m_right(1, 0, 0),
						movementSpeed(1)
{
	TMatrixIdentity(m_viewMatrix);
	TMatrixIdentity(m_projectionMatrix);
	TMatrixIdentity(m_rotationMatrix);
	TMatrixIdentity(worldToCamera);

	movementToggles[0] = 0;
	movementToggles[1] = 0;
	movementToggles[2] = 0;
	movementToggles[3] = 0;
}

XMCamera::~XMCamera()
{

}

bool XMCamera::Init(unsigned int clientWidth, unsigned int clientHeight)
{
	SetPerspectiveProjectionLH(45.0f, static_cast<float>(clientWidth), static_cast<float>(clientHeight), 1.0f, 10000.0f);
	return true;
}

void XMCamera::SetPositionAndView(float x, float y, float z, float hDeg, float pDeg)
{
	m_position = FVEC3(x, y, z);

	AdjustHeadingPitch(hDeg * (float)DEG_TO_RAD, pDeg * (float)DEG_TO_RAD);

	UpdateView();
}

void XMCamera::SetPerspectiveProjectionLH(float fov, float width, float height, float zNear, float zFar)
{
	float f = fov * (float)DEG_TO_RAD;
	// projection matrix
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fov * (float)DEG_TO_RAD, width / height, zNear, zFar);
	XMFLOAT4 p(m_position.x, m_position.y, m_position.z, 1);
	XMFLOAT4 l(m_lookAt.x, m_lookAt.y, m_lookAt.z, 1);
	XMFLOAT4 u(m_up.x, m_up.y, m_up.z, 1);

	m_viewMatrix = XMMatrixLookAtLH(XMLoadFloat4(&p), XMLoadFloat4(&l), XMLoadFloat4(&u));

	m_focalLength = m_projectionMatrix.m00;
	//m_projectionMatrix	= TMatrixTranspose(m_projectionMatrix);
	//m_viewMatrix		= TMatrixTranspose(m_viewMatrix);

	AdjustHeadingPitch(0, 0);
}

void XMCamera::UpdateView()
{
	// rotate
	m_rotationMatrix = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, m_roll);

	// transform
	const FVEC3 dv(0, 0, 1);
	const FVEC3 du(0, 1, 0);

	m_lookAt = TVectorTransform(dv, m_rotationMatrix);
	m_up = TVectorTransform(du, m_rotationMatrix);

	// forward & right
	m_forward	= Normalize(m_lookAt);
	m_right		= Cross(m_up, m_forward);
	m_right		= Normalize(m_right);


	// update lookAt
	m_lookAt = m_position + m_lookAt;

	// update viewMatrix
	XMFLOAT3 p(m_position.x, m_position.y, m_position.z);
	XMFLOAT3 l(m_lookAt.x, m_lookAt.y, m_lookAt.z);
	XMFLOAT3 u(m_up.x, m_up.y, m_up.z);

	m_viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&p), XMLoadFloat3(&l), XMLoadFloat3(&u));
	//m_viewMatrix = TMatrixTranspose(m_viewMatrix);

	// update inverse matrix
	m_inverseViewMatrix = XMMatrixInverse(&XMMatrixDeterminant(m_viewMatrix), m_viewMatrix);
	m_inverseProjectionMatrix = XMMatrixInverse(&XMMatrixDeterminant(m_projectionMatrix), m_projectionMatrix);
	/* (world) view projection */
	//m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	m_viewProjectionMatrix	= m_projectionMatrix * m_viewMatrix;
	m_inverseWVP			= XMMatrixInverse(&XMMatrixDeterminant(m_viewProjectionMatrix), m_viewProjectionMatrix);

	
}

void XMCamera::Update(const Time& time)
{
	float deltaTime = time.dtMS;

	//FVEC3 dir = m_forward * movementSpeed * (movementToggles[0] + movementToggles[1]) + m_right * movementSpeed * (movementToggles[2] + movementToggles[3]);
	FVEC3 dir = m_forward * (deltaTime * (movementToggles[0] + movementToggles[1]) * movementSpeed) +
		m_right	  * (deltaTime * (movementToggles[2] + movementToggles[3]) * movementSpeed);

	m_position = m_position + dir;

	UpdateView();
}

void XMCamera::SetMovementToggle(int i, int v)
{
	movementToggles[i] = v;
}

void XMCamera::SetMovementSpeed(const float _unitsPerMS)
{
	movementSpeed = _unitsPerMS;
}

void XMCamera::AdjustHeadingPitch(float hRad, float pRad)
{
	m_yaw += hRad;
	m_pitch += pRad;

	//value clamping - keep heading and pitch between 0 and 2 pi
	if (m_yaw > TWO_PI)		m_yaw -= (float)TWO_PI;
	else if (m_yaw < 0.f)	m_yaw = (float)TWO_PI + m_yaw;

	if (m_pitch > TWO_PI)	m_pitch -= (float)TWO_PI;
	else if (m_pitch < 0.f) m_pitch = (float)TWO_PI + m_pitch;
}