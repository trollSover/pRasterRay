#pragma once

#include "../Render/Renderer.h"

class DebugRenderer
	: public IRenderer
{
private:
	Shader*	m_pVertexShaderStage;
	Shader* m_pPixelShaderStage;

	//D3DBuffer* m_debugBuffer;
	ID3D11RenderTargetView*	m_pDeferredRenderTargets[RENDERTARGET_COUNT];

protected:
public:

private:
	bool VInit(DXDriver* _driver) final;
	void VDraw(DXDriver* _driver, D3DBuffer* _model, D3DBuffer* _indices = nullptr)	final;

protected:
public:
	DebugRenderer();
	~DebugRenderer();
};