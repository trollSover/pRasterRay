#include "DXDriver.h"

#include "Core.h"
#include "DXDescs.h"
#include "../Render/Shader.h"

extern Core*			g_pCoreSystem;

# define M_PI           3.14159265358979323846

const char* DEFAULT_RTS[] = { "Color", "Normal", "Depth" };

DXDriver::DXDriver() :	m_context(0), m_device(0),
						m_swapChain(0),
						m_depthStencilBuffer(0), m_depthStencilState(0), m_depthStencilView(0),
						m_rasterState(0), m_blendState(0), m_pbackBufferRenderTarget(0)
{

}

DXDriver::~DXDriver()
{
	if (m_swapChain) 
		m_swapChain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(m_blendState);
	SAFE_RELEASE(m_rasterState);
	SAFE_RELEASE(m_depthStencilView);
	SAFE_RELEASE(m_depthStencilState);
	SAFE_RELEASE(m_depthStencilBuffer);
	SAFE_RELEASE(m_pDepthStateDisabled);

	SAFE_DELETE(m_pbackBufferRenderTarget);

	m_pRenderTargets.clear();

	SAFE_RELEASE(m_context);
	SAFE_RELEASE(m_device);
	SAFE_RELEASE(m_swapChain);

	SAFE_DELETE_ARRAY(m_videoCardDescription);
}

bool DXDriver::Init(Resolution _res)
{
	HRESULT result;
	RenderTargetInit rti;

	D3D11_VIEWPORT viewport;

	// Query DXGI about system
	m_videoCardDescription = VNEW char[128];
	result = QueryDXGI(_res, m_videoCardMemory, m_videoCardDescription);
	if (FAILED(result))
		return false;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = GetSwapChainDesc(_res, true, g_pCoreSystem->GetHwnd());
	UINT deviceFlags = 0;
#ifdef _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevelsToTry[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = sizeof(driverTypes) / sizeof(driverTypes[0]);
	D3D_DRIVER_TYPE driverType;
	D3D_FEATURE_LEVEL initiatedFeatureLevel;

	for (uint i = 0; i < numDriverTypes; ++i)
	{
		driverType = driverTypes[i];

		result = D3D11CreateDeviceAndSwapChain(NULL, driverType, NULL, deviceFlags, featureLevelsToTry, ARRAYSIZE(featureLevelsToTry),
			D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, &initiatedFeatureLevel, &m_context);

		if (SUCCEEDED(result))
		{
			D3D_FEATURE_LEVEL_11_1 == initiatedFeatureLevel ? printf("feature level: D3D_11_1\n") : printf("feature level: D3D_11_0\n");
			0 == i ? printf("driver type: hardware\n") : printf("driver type: reference\n");
			break;
		}
	}

	if (FAILED(result))
	{
		_com_error err(result);
		PrintError(AT, err.ErrorMessage());
		return false;
	}

	// create backbuffer render target
	m_pbackBufferRenderTarget = VNEW RenderTarget("BackBuffer");

	// Get the pointer to the back buffer texture from the swapchain
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&m_pbackBufferRenderTarget->m_pTexture);
	if (FAILED(result))
	{
		return false;
	}

	m_pbackBufferRenderTarget->m_pTexture->GetDesc(&rti.TexDesc);
	rti.RTVDesc.Format				= rti.TexDesc.Format;
	rti.RTVDesc.ViewDimension		= D3D11_RTV_DIMENSION_TEXTURE2D;
	rti.RTVDesc.Texture2D.MipSlice	= 0;
	rti.formats						= RTFS_TEXTURE;

	if (false == m_pbackBufferRenderTarget->Init(rti, m_device))
	{
		PrintError(AT, "failed to create backbuffer rendertarget");
		return false;
	}

	// Create the texture for the depth buffer using the filled out description.
	D3D11_TEXTURE2D_DESC depthBufferDesc = GetDepthBufferDesc(_res);
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Create the depth stencil state.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = GetDepthStencilDesc();
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	depthStencilDesc.DepthEnable = false;
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_pDepthStateDisabled);
	if (FAILED(result))
	{
		return false;
	}

	// Create the depth stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = GetDepthStencilViewDesc();
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	// Create the rasterizer state from the description we just filled out.
	D3D11_RASTERIZER_DESC rasterDesc = GetRasterizerDesc();
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	// Set the rasterizer state.
	m_context->RSSetState(m_rasterState);

	D3D11_BLEND_DESC blendDesc = GetBlendDesc(RENDERTARGET_COUNT);
	result = m_device->CreateBlendState(&blendDesc, &m_blendState);

	if (FAILED(result))
	{
		return false;
	}

	// Setup the viewport for rendering.
	viewport.Width		= (float)_res.width;
	viewport.Height		= (float)_res.height;
	viewport.MinDepth	= 0.0f;
	viewport.MaxDepth	= 1.0f;
	viewport.TopLeftX	= 0.0f;
	viewport.TopLeftY	= 0.0f;
	
	float fieldOfView	= (float)M_PI / 4.0f;
	float screenAspect	= (float)_res.width / (float)_res.height;

	m_context->RSSetViewports(1, &viewport);

#ifdef FDEBUG
	printf("Driver: Ok\n");
#endif

	return true;
}

void DXDriver::BeginScene()
{
	const float bgcolor[4] = { 0.f, 0.f, 0.f, 0.0f };

	ID3D11RenderTargetView* backBufferPtr;
	m_pbackBufferRenderTarget->GetView(backBufferPtr);
	m_context->ClearRenderTargetView(backBufferPtr, bgcolor);
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_context->OMSetDepthStencilState(m_depthStencilState, 1);

	m_context->RSSetState(m_rasterState);

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_context->OMSetBlendState(m_blendState, blendFactor, 0xffffffff);
}

bool DXDriver::EndScene()
{
	HRESULT hr = S_OK;

	// present final image
	hr = m_swapChain->Present(0, 0);

	if (FAILED(hr))
	{
		PrintError(AT, "swapchain failed");
		return false;
	}

	return true;
}

void DXDriver::SetRasterizerState(D3D11_CULL_MODE _cullMode, D3D11_FILL_MODE _fillMode)
{
	D3D11_RASTERIZER_DESC rasterDesc = GetRasterizerDesc(_cullMode, _fillMode);
	HRESULT hr = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);

	if (FAILED(hr))
	{
		PrintError(AT, "failed to set new rasterizer state");
	}

	printf("rasterizer state updated\n");
	// Set the rasterizer state.
	m_context->RSSetState(m_rasterState);
}

bool DXDriver::GetRenderTarget(const unsigned int _id, RenderTarget*& _renderTarget)
{
	if (!m_pRenderTargets.count(_id))
		return false;

	_renderTarget = m_pRenderTargets.at(_id);

	return true;
}

bool DXDriver::CreateRenderTarget(const unsigned int _id, const RenderTargetInit& _rti, RenderTarget* _renderTargetPtr)
{
	// Id already used
	if (m_pRenderTargets[_id] != nullptr)
		return false;

	RenderTarget* temp = VNEW RenderTarget(std::to_string(_id).c_str());

	// Failed to allocate mem
	if (!temp)
		return false;

	// Failed to initialize, delete allocated mem
	if (!temp->Init(_rti, m_device))
	{
		SAFE_DELETE(temp);
		return false;
	}

	// all is good
	m_pRenderTargets[_id]	= temp;
	_renderTargetPtr		= temp;

	return true;
}

void DXDriver::SetRenderTargets(ID3D11RenderTargetView** _renderTargets, const unsigned int _count)
{
	m_context->OMSetRenderTargets(_count, &_renderTargets[0], m_depthStencilView);
}

void DXDriver::ClearRenderTargets(ID3D11RenderTargetView** _renderTargets, const unsigned int _count)
{
	static const float color[4] = { 0.f, 0.f, 0.f, 0.f };
	for (unsigned int i = 0; i < _count; ++i)
	{
		m_context->ClearRenderTargetView(_renderTargets[i], color);
	}
}