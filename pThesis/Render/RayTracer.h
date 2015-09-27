#pragma once

#include "Renderer.h"

class RayTracer
	: public IRenderer
{
private:
	Shader*	m_pPrimRayStage;
	Shader* m_pIntersectionStage;
	Shader* m_pColorStage;

	D3DBuffer*	m_pRays;
	D3DBuffer*	m_pColorAccumulation;
	D3DBuffer*	m_pIntersections;

protected:
public:

private:
	bool VInit(DXDriver* _driver) final;
	void VDraw(DXDriver* _driver, D3DBuffer* _model, D3DBuffer* _indices = nullptr)	final;

protected:
public:
	RayTracer();
	~RayTracer();
};