#pragma once

#include "Renderer.h"
//#include "Font.h"

class Overlay
	: public IRenderer
{
private:
	//Font*	m_pFont;
	Shader*		m_pVertexShaderStage;
	Shader*		m_pPixelShaderStage;

	D3DBuffer*	m_pTexture;
	//ID3D11ShaderResourceView*	m_pTexture;
protected:
public:

private:
protected:
public:
	Overlay(void);
	~Overlay(void);

	bool VInit(DXDriver* _driver) final;
	void VDraw(DXDriver* _driver, D3DBuffer* _model);
};