#pragma once

#include "Renderer.h"

class BackBufferRender
	: public IRenderer
{
private:
	Shader						*m_pBackBufferShaderVS;
	Shader						*m_pBackBufferShaderPS;
	ID3D11ShaderResourceView	*m_pShaderResources[RENDERTARGET_COUNT];
	ID3D11RenderTargetView		*m_pBackBufferRTV;
	ID3D11SamplerState			*m_pPointSampler;

protected:
public:

private:
	bool VInit(DXDriver* _driver) final;
	void VDraw(DXDriver* _driver, D3DBuffer* _model, D3DBuffer* _indices = nullptr)	final;

	bool FetchShaderResource(const unsigned int _Id, DXDriver* _driver, ID3D11ShaderResourceView*& _resource);

protected:
public:
	BackBufferRender();
	~BackBufferRender();
};