#pragma once

#include "Renderer.h"
#include "../CoreSystems/CoreStd.h"
#include "../CoreSystems/DXDriver.h"

#include "Shader.h"
#include "Render_Create.h"

class Rasterizer
	: public IRenderer
{
private:
	Shader*		m_pVertexShaderStage;
	Shader*		m_pGeometryShaderStage;
	Shader*		m_pPixelShaderStage;

	ID3D11RenderTargetView*	m_pDeferredRenderTargets[RENDERTARGET_COUNT];

protected:
public:

private:
	bool VInit(DXDriver* _driver) final;
	void VDraw(DXDriver* _driver, D3DBuffer* _model, D3DBuffer* _indices = nullptr)	final;

protected:
public:
	Rasterizer();
	~Rasterizer();
};

