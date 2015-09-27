#include "DXCamera.h"

#define TWO_PI		6.283185307179586476925286766559
#define DEG_TO_RAD	0.01745329251994329576923690768489

DXCamera::DXCamera() : TCamera()
{
	m_iPosition = XMFLOAT3(m_position.x, m_position.y, m_position.z);
	m_iTarget	= XMFLOAT3(m_target.x, m_target.y, m_target.z);
	m_iUp		= XMFLOAT3(m_up.x, m_up.y, m_up.z);

	XMStoreFloat4x4(&m_iView, XMMatrixIdentity());
	XMStoreFloat4x4(&m_iProj, XMMatrixIdentity());
}

bool DXCamera::VInternalInit()
{
	XMStoreFloat4x4(&m_iView, XMMatrixLookAtLH(XMLoadFloat3(&m_iPosition), XMLoadFloat3(&m_iTarget), XMLoadFloat3(&m_iUp)));
	XMStoreFloat4x4(&m_iProj, XMMatrixPerspectiveFovLH(m_fov * (float)(DEG_TO_RAD), m_clientWidth / m_clientHeight, m_nearClip, m_farClip));

	return true;
}

void DXCamera::VMove(const FVEC3& _dir)
{
	XMFLOAT3 dir = XMFLOAT3(_dir.x, _dir.y, _dir.z);

	XMVECTOR p = XMVector3Transform(XMLoadFloat3(&m_iPosition), XMMatrixTranslation(dir.x, dir.y, dir.z));
	XMStoreFloat3(&m_iPosition, p);
	m_position = p;

	XMVECTOR t = XMVector3Transform(XMLoadFloat3(&m_iTarget), XMMatrixTranslation(dir.x, dir.y, dir.z));
	XMStoreFloat3(&m_iTarget, t);
	m_target = t;

	XMVECTOR u = XMVector3Transform(XMLoadFloat3(&m_iUp), XMMatrixTranslation(dir.x, dir.y, dir.z));
	XMStoreFloat3(&m_iUp, u);
	m_up = u;
}

void DXCamera::VRotate(const FVEC3& _axis, float _degrees)
{
	XMFLOAT3 axis = XMFLOAT3(_axis.x, _axis.y, _axis.z);

	if (XMVector3Equal(XMLoadFloat3(&axis), XMVectorZero()) || _degrees == 0)
		return;

	XMFLOAT3 lookAtTarget, lookAtUp;
	XMStoreFloat3(&lookAtTarget, XMLoadFloat3(&m_iTarget) - XMLoadFloat3(&m_iPosition));
	XMStoreFloat3(&lookAtUp, XMLoadFloat3(&m_iUp) - XMLoadFloat3(&m_iPosition));

	XMVECTOR vLookAtT = XMVector3Transform(XMLoadFloat3(&lookAtTarget), XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(_degrees)));
	XMVECTOR vLookAtU = XMVector3Transform(XMLoadFloat3(&lookAtUp), XMMatrixRotationAxis( XMLoadFloat3(&axis), XMConvertToRadians(_degrees)));

	XMStoreFloat3(&m_iTarget, XMLoadFloat3(&m_iPosition) + vLookAtT);
	XMStoreFloat3(&m_iUp, XMLoadFloat3(&m_iPosition) + vLookAtU);

	XMStoreFloat4x4(&m_iView, XMMatrixLookAtLH(XMLoadFloat3(&m_iPosition), XMLoadFloat3(&m_iTarget), XMLoadFloat3(&m_iUp)));
}

void DXCamera::VSetPosition(const FVEC3& _pos)
{
	XMFLOAT3 pos = XMFLOAT3(_pos.x, _pos.y, _pos.z);
	XMStoreFloat3(&pos, XMLoadFloat3(&pos) - XMLoadFloat3(&m_iPosition));
	XMFLOAT3 target = m_iTarget;

	FVEC3 vdir = FVEC3(pos.x, pos.y, pos.z);
	VMove(vdir);
	
}

void DXCamera::VSetTarget(const FVEC3& _target)
{
	XMFLOAT3 target = XMFLOAT3(_target.x, _target.y, _target.z);
	if (XMVector3Equal(XMLoadFloat3(&target), XMLoadFloat3(&m_iPosition)) ||
		XMVector3Equal(XMLoadFloat3(&target), XMLoadFloat3(&m_iTarget)))
		return;

	XMFLOAT3 oldTarget, newTarget;
	XMStoreFloat3(&oldTarget, XMLoadFloat3(&m_iTarget) - XMLoadFloat3(&m_iPosition));
	XMStoreFloat3(&newTarget, XMLoadFloat3(&target) - XMLoadFloat3(&m_iPosition));

	float angle = XMConvertToDegrees(XMVectorGetX(XMVector3AngleBetweenNormals(XMVector3Normalize(XMLoadFloat3(&oldTarget)), XMVector2Normalize(XMLoadFloat3(&newTarget)))));
	if (angle != 0.0f && angle != 360.0f && angle != 180.0f)
	{
		XMFLOAT3 axis;
		XMStoreFloat3(&axis,XMVector3Cross(XMLoadFloat3(&oldTarget), XMLoadFloat3(&newTarget)));
		FVEC3 vaxis = FVEC3(axis.x, axis.y, axis.z);
		VRotate(vaxis, angle);
	}
	m_iTarget = newTarget;
	XMStoreFloat4x4(&m_iView, XMMatrixLookAtLH(XMLoadFloat3(&m_iPosition), XMLoadFloat3(&m_iTarget), XMLoadFloat3(&m_iUp)));
}