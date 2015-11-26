#pragma once

#include <Windows.h>

#include "CoreStd.h"
#include "../Render/RenderTarget.h"
#include "../Application/PerfTracker.h"

class Shader;

static const int RENDERTARGET_COUNT = 3;

//#include <hash_map>
#include <unordered_map>

enum RETURNSTATE
{
	RS_OK			= 0,
	RS_MINOR_ERROR	= 0x1,
	RS_FATAL_ERROR	= 0x2,
	RS_CUSTOM_ERROR = 0x4	// and above in 2^N pattern
};

enum RENDERTARGETS
{
	RT_NONE,
	RT_BACKBUFFER,
	RT_COLOR,
	RT_NORMAL,
	RT_DEPTH
};

class DXDriver
{
private:
	AmdPerf::PerfLibrary	m_perfLib;

	bool	m_vsync_enabled;
	int		m_videoCardMemory;
	char*	m_videoCardDescription;

	ID3D11Device*			m_device;
	ID3D11DeviceContext*	m_context;
	IDXGISwapChain*			m_swapChain;

	ID3D11Texture2D*			m_depthStencilBuffer;
	ID3D11DepthStencilState*	m_depthStencilState;
	ID3D11DepthStencilView*		m_depthStencilView;

	ID3D11DepthStencilState* m_pDepthStateDisabled;

	ID3D11RasterizerState*	m_rasterState;
	ID3D11BlendState*		m_blendState;

	RenderTarget *m_pbackBufferRenderTarget;

	//std::hash_map<const char*, RenderTarget*> m_pRenderTargets;
	std::unordered_map<unsigned int, RenderTarget*> m_pRenderTargets;

protected:
public:

private:
protected:
public:
	AmdPerf::PerfLibrary* GetPerfLibrary(void) { return &m_perfLib; }

	DXDriver();
	~DXDriver();

	bool Init(Resolution _res);

	void BeginScene();
	bool EndScene();

	void SetRasterizerState(D3D11_CULL_MODE _cullMode, D3D11_FILL_MODE _fillMode);
	
	ID3D11Device*			GetDevice()		{ return m_device; };
	ID3D11DeviceContext*	GetContext()	{ return m_context; };
	RenderTarget*			GetBackBuffer()	{ return m_pbackBufferRenderTarget; }

	bool GetRenderTarget(const unsigned int _id, RenderTarget*& _renderTarget);
	bool CreateRenderTarget(const unsigned int _id, const RenderTargetInit& _rti, RenderTarget* _renderTarget = nullptr);

	void SetRenderTargets(ID3D11RenderTargetView** _renderTargets, const unsigned int _count);
	void ClearRenderTargets(ID3D11RenderTargetView** _renderTargets, const unsigned int _count);

	void SetZBufferOn()  { m_context->OMSetDepthStencilState(m_depthStencilState, 1); }
	void SetZBufferOff() { m_context->OMSetDepthStencilState(m_pDepthStateDisabled, 1); }

	template<typename T>
	HRESULT MapSubResource(ID3D11Resource* _resource, T& _data, D3D11_MAP _mapType = D3D11_MAP_WRITE_DISCARD)
	{
		HRESULT hr = S_OK;
		D3D11_MAPPED_SUBRESOURCE msr;
		hr = m_context->Map(_resource, 0, _mapType, 0, &msr);
		if (FAILED(hr))	
			return hr;
		T* tmp	= reinterpret_cast<T*>(msr.pData);
		tmp[0]	= _data;
		tmp		= nullptr;
		m_context->Unmap(_resource, 0);
		return hr;
	}
};