#pragma once

#include "TCamera.h"

class DXCamera
	: public TCamera
{
	/* Members */
private:
	/* Internal Camera params */
	XMFLOAT3	m_iPosition,
				m_iTarget,
				m_iUp;

	/* Internal Matrices */
	XMFLOAT4X4	m_iView,
				m_iProj,
				m_iVP,
				m_iOrtho,
				m_iInverse;
protected:
public:

	/* Methods */
private:


protected:
public:
	bool VInternalInit() final;

	void VMove(const FVEC3& _dir) final;
	void VRotate(const FVEC3& _axis, float _degrees) final;

	void VSetPosition(const FVEC3& _pos)  final;
	void VSetTarget(const FVEC3& _target) final;

	DXCamera();
	~DXCamera() { }

	XMFLOAT4X4 GetViewMatrix()			{ return m_iView; }
	XMFLOAT4X4 GetProjectionMatrix()	{ return m_iProj; }
	XMFLOAT4X4 GetOrthographicMatrix()	{ return m_iOrtho; }
	XMFLOAT4X4 GetInverseVPMatrix()		{ return m_iInverse; }
	XMFLOAT4X4 GetVPMatrix()			{ return m_iVP; }

};