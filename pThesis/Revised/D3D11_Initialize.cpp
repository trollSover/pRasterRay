#include "RasterRayApp.h"
#include <iostream>
#include <fstream>

using namespace RasterRay;

HRESULT RasterRayApp::CreateDeviceAndContext(void)
{
	HRESULT hr = S_OK;

	/* Create device and context */
	unsigned deviceFlags = 0;
#if defined _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };	// d3d gets cranky if not array
	const D3D_DRIVER_TYPE	driverType = D3D_DRIVER_TYPE_HARDWARE;

	hr = D3D11CreateDevice(0, driverType, 0, deviceFlags, featureLevels, 1, D3D11_SDK_VERSION, &m_pDevice, 0, &m_pContext);

	if (FAILED(hr))
	{
		PrintError(AT, hr);
	}

	return hr;
}

HRESULT RasterRayApp::CreateRasterStates(void)
{
	HRESULT hr = S_OK;
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
	desc.CullMode			= D3D11_CULL_NONE;
	desc.FillMode			= D3D11_FILL_SOLID;
	desc.DepthClipEnable	= true;
	hr = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerStates[RSI_CNFS].m_pState);
	if (FAILED(hr)) return hr;
	m_pRasterizerStates[RSI_CNFS].cull = "none";
	m_pRasterizerStates[RSI_CNFS].fill = "solid";


	desc.CullMode			= D3D11_CULL_NONE;
	desc.FillMode			= D3D11_FILL_WIREFRAME;
	desc.DepthClipEnable	= true;
	hr = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerStates[RSI_CNFW].m_pState);
	if (FAILED(hr)) return hr;
	m_pRasterizerStates[RSI_CNFW].cull = "none";
	m_pRasterizerStates[RSI_CNFW].fill = "wireframe";


	desc.CullMode			= D3D11_CULL_BACK;
	desc.FillMode			= D3D11_FILL_SOLID;
	desc.DepthClipEnable	= true;
	hr = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerStates[RSI_CBFS].m_pState);
	if (FAILED(hr)) return hr;
	m_pRasterizerStates[RSI_CBFS].cull = "back";
	m_pRasterizerStates[RSI_CBFS].fill = "solid";


	desc.CullMode			= D3D11_CULL_BACK;
	desc.FillMode			= D3D11_FILL_WIREFRAME;
	desc.DepthClipEnable	= true;
	hr = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerStates[RSI_CBFW].m_pState);
	if (FAILED(hr)) return hr;
	m_pRasterizerStates[RSI_CBFW].cull = "back";
	m_pRasterizerStates[RSI_CBFW].fill = "wireframe";

	desc.CullMode			= D3D11_CULL_FRONT;
	desc.FillMode			= D3D11_FILL_SOLID;
	desc.DepthClipEnable	= true;
	hr = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerStates[RSI_CFFS].m_pState);
	if (FAILED(hr)) return hr;
	m_pRasterizerStates[RSI_CFFS].cull = "front";
	m_pRasterizerStates[RSI_CFFS].fill = "solid";

	desc.CullMode			= D3D11_CULL_FRONT;
	desc.FillMode			= D3D11_FILL_WIREFRAME;
	desc.DepthClipEnable	= true;
	hr = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerStates[RSI_CFFW].m_pState);
	if (FAILED(hr)) return hr;
	m_pRasterizerStates[RSI_CFFW].cull = "front";
	m_pRasterizerStates[RSI_CFFW].fill = "wireframe";


	return S_OK;
}

HRESULT RasterRayApp::CompileShader(LPCWSTR _file, LPCSTR _type, ID3D10Blob*& _shaderByteCode, LPCSTR _kernel, D3D_SHADER_MACRO* _pShaderDefines)
{
	HRESULT hr			= S_OK;
	ID3DBlob* error		= nullptr;
	FILE* shaderBlob	= nullptr;
	DWORD dwsf			= D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS | D3DCOMPILE_PREFER_FLOW_CONTROL;
#ifdef _DEBUG
	dwsf |= D3DCOMPILE_DEBUG;
	dwsf |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	dwsf |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
	//dwsf |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	
	hr = D3DCompileFromFile(_file, _pShaderDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, _kernel, _type, dwsf, NULL, &_shaderByteCode, &error);
	
	if (error) { PrintError(AT, (const char*)error->GetBufferPointer()); return E_FAIL; }
	if (FAILED(hr))	{ PrintError(AT, hr); }

	return hr;
}

HRESULT RasterRayApp::CreateBackBufferEnvironment(const DXGI_FORMAT _format)
{
	HRESULT hr = S_OK;

	/************************
	--- CREATE BACKBUFFER ---
	*************************/

	/* set backbuffer texture */
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&m_pBackBufferTexture);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/* create backbuffer rendertarget */
	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	ZeroMemory(&rtvd, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvd.Format = _format;
	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	hr = m_pDevice->CreateRenderTargetView(m_pBackBufferTexture, &rtvd, &m_pBackBufferRenderTargetView);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/***************************
	--- CREATE DEPTH STENCIL ---
	****************************/

	/* create backbuffer depth texture */
	D3D11_TEXTURE2D_DESC td = { 0 };
	td.Width				= VGetResolution().width;
	td.Height				= VGetResolution().height;
	td.MipLevels			= 1;
	td.ArraySize			= 1;
	td.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	td.SampleDesc.Count		= 1;
	td.Usage				= D3D11_USAGE_DEFAULT;
	td.BindFlags			= D3D11_BIND_DEPTH_STENCIL;

	hr = m_pDevice->CreateTexture2D(&td, 0, &m_pDepthStencilTexture);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/* create depth stencil states */
	D3D11_DEPTH_STENCIL_DESC dsd		= { 0 };
	dsd.DepthEnable						= true;
	dsd.DepthWriteMask					= D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc						= D3D11_COMPARISON_LESS;
	dsd.StencilEnable					= true;
	dsd.StencilReadMask					= 0xFF;
	dsd.StencilWriteMask				= 0xFF;
	dsd.FrontFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_INCR;
	dsd.FrontFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
	dsd.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
	dsd.BackFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilDepthFailOp		= D3D11_STENCIL_OP_DECR;
	dsd.BackFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
	dsd.BackFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

	/* depth enabled */
	hr = m_pDevice->CreateDepthStencilState(&dsd, &m_pDepthStencilStates[DSSI_ENABLED]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/* depth disabled*/
	dsd.DepthEnable = false;
	hr = m_pDevice->CreateDepthStencilState(&dsd, &m_pDepthStencilStates[DSSI_DISABLED]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	//m_pCurrentDepthStencilState = m_pDepthStencilEnabled;

	/* create depth stencil view */
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	dsvd.Format			= DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvd.ViewDimension	= D3D11_DSV_DIMENSION_TEXTURE2D;

	hr = m_pDevice->CreateDepthStencilView(m_pDepthStencilTexture, &dsvd, &m_pDepthStencilView);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/***************************
	--- CREATE RASTER STATES ---
	****************************/

	hr = CreateRasterStates();
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }
	//m_pCurrentRasterizerState = m_pRasterizerStates[RSI_CFFS];

	/****************************
	--- CREATE SAMPLER STATES ---
	*****************************/

	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(D3D11_SAMPLER_DESC));
	sd.Filter			= D3D11_FILTER_MIN_MAG_MIP_POINT;
	sd.AddressU			= D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV			= D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW			= D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.MipLODBias		= 0.0f;
	sd.MaxAnisotropy	= 1;
	sd.ComparisonFunc	= D3D11_COMPARISON_ALWAYS;
	sd.BorderColor[0]	= 0;
	sd.BorderColor[1]	= 0;
	sd.BorderColor[2]	= 0;
	sd.BorderColor[3]	= 0;
	sd.MinLOD			= 0;
	sd.MaxLOD			= D3D11_FLOAT32_MAX;

	m_pDevice->CreateSamplerState(&sd, &m_pPointSampler);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/**************************
	--- CREATE BLEND STATES ---
	***************************/

	D3D11_BLEND_DESC bd			= { 0 };
	bd.AlphaToCoverageEnable	= false;
	bd.IndependentBlendEnable	= false;
	for (unsigned i = 0; i < RENDERTARGET_COUNT; ++i)
	{
		bd.RenderTarget[i].BlendEnable				= false;
		bd.RenderTarget[i].SrcBlend					= D3D11_BLEND_ONE;
		bd.RenderTarget[i].DestBlend				= D3D11_BLEND_ONE;
		bd.RenderTarget[i].BlendOp					= D3D11_BLEND_OP_ADD;
		bd.RenderTarget[i].SrcBlendAlpha			= D3D11_BLEND_ONE;
		bd.RenderTarget[i].DestBlendAlpha			= D3D11_BLEND_ZERO;
		bd.RenderTarget[i].BlendOpAlpha				= D3D11_BLEND_OP_ADD;
		bd.RenderTarget[i].RenderTargetWriteMask	= 0x0f;
	}

	m_pDevice->CreateBlendState(&bd, &m_pBlendState);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/**********************
	--- CREATE VIEWPORT ---
	***********************/

	m_viewPort.Width	= VGetResolution().width;
	m_viewPort.Height	= VGetResolution().height;
	m_viewPort.MinDepth = 0.f;
	m_viewPort.MaxDepth = 1.f;
	m_viewPort.TopLeftX = 0.f;
	m_viewPort.TopLeftY = 0.f;

	m_pContext->RSSetViewports(1, &m_viewPort);
	
	/*********************
	--- CREATE SHADERS ---
	**********************/

	/* --- BackBuffer Shader --- */
	/* Vertex shader */
	ID3D10Blob* pShaderByteCode;
	hr = CompileShader(L"Revised/Shaders/DeferredBackBuffer.hlsl", "vs_5_0", pShaderByteCode, "MainVS");
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateVertexShader(pShaderByteCode->GetBufferPointer(), pShaderByteCode->GetBufferSize(), 0, &m_pBackBufferVertexShader);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/* Input layout */
	D3D11_INPUT_ELEMENT_DESC ld[2] = { 0, 0 };
	ld[0].SemanticName		= "POSITION";
	ld[0].Format			= DXGI_FORMAT_R32G32B32A32_FLOAT;
	ld[0].InputSlotClass	= D3D11_INPUT_PER_VERTEX_DATA;
	ld[1].SemanticName		= "TEXCOORD";
	ld[1].Format			= DXGI_FORMAT_R32G32_FLOAT;
	ld[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	ld[1].InputSlotClass	= D3D11_INPUT_PER_VERTEX_DATA;

	hr = m_pDevice->CreateInputLayout(ld, 2, pShaderByteCode->GetBufferPointer(), pShaderByteCode->GetBufferSize(), &m_pBackBufferInputLayout);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	pShaderByteCode->Release();

	/* Pixel shader */
	hr = CompileShader(L"Revised/Shaders/DeferredBackBuffer.hlsl", "ps_5_0", pShaderByteCode, "MainPS");
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreatePixelShader(pShaderByteCode->GetBufferPointer(), pShaderByteCode->GetBufferSize(), 0, &m_pBackBufferPixelShader);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	pShaderByteCode->Release();

	/***********************************************
	--- CREATE GLOBAL CONSTANT BUFFER STRUCTURES ---
	************************************************/

	/* Camera Matrices for shaders */
	D3D11_BUFFER_DESC cbbd	= { 0 };
	cbbd.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	cbbd.Usage				= D3D11_USAGE_DYNAMIC;
	cbbd.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	cbbd.ByteWidth			= sizeof(CameraConstantBuffer);

	hr = m_pDevice->CreateBuffer(&cbbd, 0, &m_pGlobalCameraConstantBuffer);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/**********************************
	--- CREATE SHADER STAGE BUFFERS ---
	***********************************/
	struct PT { FVEC4 p; FVEC2 t; PT(FVEC4 _p, FVEC2 _t) : p(_p), t(_t) { } };
	const PT Quad[] = { PT({ -1, -1, 0, 1 }, { 0, 1 }), 
						PT({ 1, -1, 0, 1 }, { 1, 1 }), 
						PT({ -1, 1, 0, 1 }, { 0, 0 }), 
						PT({ 1, 1, 0, 1 }, { 1, 0 }) };

	const unsigned Order[] = { 0, 2, 1, 2, 3, 1 };

	D3D11_BUFFER_DESC dbd = { 0 };
	
	/* Vertex buffer */
	dbd.BindFlags			= D3D11_BIND_VERTEX_BUFFER;
	dbd.Usage				= D3D11_USAGE_IMMUTABLE;
	dbd.ByteWidth			= sizeof(PT) * 4;
	dbd.StructureByteStride = sizeof(PT);
	
	D3D11_SUBRESOURCE_DATA dsrd = { 0 };
	dsrd.pSysMem				= &Quad[0];

	hr = m_pDevice->CreateBuffer(&dbd, &dsrd, &m_pBackBufferVertexBuffer);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	dbd.BindFlags			= D3D11_BIND_INDEX_BUFFER;
	dbd.ByteWidth			= sizeof(unsigned) * 6;
	dbd.StructureByteStride = sizeof(unsigned);
	dsrd.pSysMem			= &Order[0];

	hr = m_pDevice->CreateBuffer(&dbd, &dsrd, &m_pBackBufferIndexBuffer);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	return S_OK;
}

HRESULT RasterRayApp::CreateDeferredEnvironment(const DXGI_FORMAT _format)
{
	HRESULT hr = S_OK;

	/********************************
	--- CREATE DEFERRED RESOURCES ---
	*********************************/

	for (unsigned i = 0; i < RENDERTARGET_COUNT; ++i)
	{
		m_pDeferredRenderTargetViews[i]		= nullptr;
		m_pDeferredShaderResourceViews[i]	= nullptr;
		m_pDeferredTextures[i]				= nullptr;
		m_pDeferredUnorderedAccessViews[i]	= nullptr;
	}

	/* deferred textures */
	D3D11_TEXTURE2D_DESC dtd = { 0 };
	dtd.Width				= VGetResolution().width;
	dtd.Height				= VGetResolution().height;
	dtd.MipLevels			= 1;
	dtd.ArraySize			= 1;
	dtd.Format				= _format;
	dtd.SampleDesc.Count	= 1;
	dtd.Usage				= D3D11_USAGE_DEFAULT;
	dtd.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	/* deferred rendertarget views */
	D3D11_RENDER_TARGET_VIEW_DESC drtvd;
	ZeroMemory(&drtvd, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	drtvd.Format		= _format;
	drtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	/* deferred shader resource views */
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format				= _format;
	dsrvd.ViewDimension			= D3D11_SRV_DIMENSION_TEXTURE2D;
	dsrvd.Texture2D.MipLevels	= 1;

	/* deferred unordered access views */
	D3D11_UNORDERED_ACCESS_VIEW_DESC duavd;
	ZeroMemory(&duavd, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	duavd.Format		= DXGI_FORMAT_UNKNOWN;
	duavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

	/* create deferred target COLOR */
	hr = m_pDevice->CreateTexture2D(&dtd, 0, &m_pDeferredTextures[DRTI_Color]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateRenderTargetView(m_pDeferredTextures[DRTI_Color], &drtvd, &m_pDeferredRenderTargetViews[DRTI_Color]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateShaderResourceView(m_pDeferredTextures[DRTI_Color], &dsrvd, &m_pDeferredShaderResourceViews[DRTI_Color]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateUnorderedAccessView(m_pDeferredTextures[DRTI_Color], &duavd, &m_pDeferredUnorderedAccessViews[DRTI_Color]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/* create deferred target NORMAL */
	hr = m_pDevice->CreateTexture2D(&dtd, 0, &m_pDeferredTextures[DRTI_Normal]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateRenderTargetView(m_pDeferredTextures[DRTI_Normal], &drtvd, &m_pDeferredRenderTargetViews[DRTI_Normal]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateShaderResourceView(m_pDeferredTextures[DRTI_Normal], &dsrvd, &m_pDeferredShaderResourceViews[DRTI_Normal]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateUnorderedAccessView(m_pDeferredTextures[DRTI_Normal], &duavd, &m_pDeferredUnorderedAccessViews[DRTI_Normal]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/* create deferred target DEPTH */
	dtd.Format = dsrvd.Format = drtvd.Format = DXGI_FORMAT_R32_FLOAT;

	hr = m_pDevice->CreateTexture2D(&dtd, 0, &m_pDeferredTextures[DRTI_Depth]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateRenderTargetView(m_pDeferredTextures[DRTI_Depth], &drtvd, &m_pDeferredRenderTargetViews[DRTI_Depth]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateShaderResourceView(m_pDeferredTextures[DRTI_Depth], &dsrvd, &m_pDeferredShaderResourceViews[DRTI_Depth]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateUnorderedAccessView(m_pDeferredTextures[DRTI_Depth], &duavd, &m_pDeferredUnorderedAccessViews[DRTI_Depth]);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/* create window attribute constant buffer */
	WindowConstantBuffer wcb		= { 0 };
	wcb.height						= VGetResolution().width;
	wcb.width						= VGetResolution().height;

	D3D11_BUFFER_DESC wcbDesc		= { 0 };
	wcbDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
	wcbDesc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
	wcbDesc.ByteWidth				= sizeof(WindowConstantBuffer);
	wcbDesc.Usage					= D3D11_USAGE_DYNAMIC;

	D3D11_SUBRESOURCE_DATA wcbRSD	= { 0 };
	wcbRSD.pSysMem					= &wcb;

	hr = m_pDevice->CreateBuffer(&wcbDesc, &wcbRSD, &m_pGlobalWindowConstantBuffer);
	if (FAILED(hr))	{ PrintError(AT, hr); return hr; }

	//D3D11_MAPPED_SUBRESOURCE msr = { 0 };

	//hr = m_pContext->Map(m_pGlobalWindowConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	//if (FAILED(hr))	{ PrintError(AT, hr); return hr; }

	//errno_t err = memcpy_s(msr.pData, msr.RowPitch, &wcb, sizeof(WindowConstantBuffer));
	//m_pContext->Unmap(m_pGlobalWindowConstantBuffer, 0);

	return S_OK;
}

HRESULT RasterRayApp::CreateRasterizer(const DXGI_FORMAT)
{
	HRESULT hr = S_OK;

	/* Vertex Shader */
	
	ID3D10Blob* pShaderByteCode = nullptr;
	hr = CompileShader(L"Revised/Shaders/RasterizeScene.hlsl", "vs_5_0", pShaderByteCode, "MainVS");
	if (hr != S_OK) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateVertexShader(pShaderByteCode->GetBufferPointer(), pShaderByteCode->GetBufferSize(), 0, &m_pRasterizerVertexShader);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/* Input layout */
	D3D11_INPUT_ELEMENT_DESC ld[3]	= { 0 };
	ld[0].SemanticName				= "POSITION";
	ld[0].Format					= DXGI_FORMAT_R32G32B32_FLOAT;
	ld[0].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
	ld[1].SemanticName				= "NORMAL";
	ld[1].Format					= DXGI_FORMAT_R32G32B32_FLOAT;
	ld[1].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
	ld[1].AlignedByteOffset			= D3D11_APPEND_ALIGNED_ELEMENT;
	ld[2].SemanticName				= "COLOR";
	ld[2].Format					= DXGI_FORMAT_R32G32B32A32_FLOAT;
	ld[2].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
	ld[2].AlignedByteOffset			= D3D11_APPEND_ALIGNED_ELEMENT;

	hr = m_pDevice->CreateInputLayout(ld, 3, pShaderByteCode->GetBufferPointer(), pShaderByteCode->GetBufferSize(), &m_pRasterizerGeometryInputLayout);
	if (hr != S_OK) { PrintError(AT, hr); return hr; }

	pShaderByteCode->Release();

	/* Pixel Shader */

	hr = CompileShader(L"Revised/Shaders/RasterizeScene.hlsl", "ps_5_0", pShaderByteCode, "MainPS");
	if (hr != S_OK) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreatePixelShader(pShaderByteCode->GetBufferPointer(), pShaderByteCode->GetBufferSize(), 0, &m_pRasterizerPixelShader);
	if (hr != S_OK) { PrintError(AT, hr); return hr; }

	pShaderByteCode->Release();
	
	return S_OK;
}

HRESULT RasterRayApp::CreateRaycaster(void)
{
	HRESULT hr = S_OK;

	/* create compute shader */
	D3D_SHADER_MACRO defines[] =
	{
		{ "STACK_LIMIT", "15" },
		{ "ITR_LIMIT", "1000" },
		{ "THREAD_COUNT_X", "32" },
		{ "THREAD_COUNT_Y", "16" },
		{ "WORK_SIZE_X", "5" },
		{ "WORK_SIZE_Y", "3" },
		NULL, NULL
	};
	ID3D10Blob* pShaderByteCode = nullptr;
	hr = CompileShader(L"Revised/Shaders/RCSinglePassRaycast.hlsl", "cs_5_0", pShaderByteCode, "MainCS", &defines[0]);
	if (hr != S_OK) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateComputeShader(pShaderByteCode->GetBufferPointer(), pShaderByteCode->GetBufferSize(), 0, &m_pRayCastComputeShader);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/* create shader node data */
	SVO_Header header = m_sparseOctree.GetHeader();


	D3D11_BUFFER_DESC volumeDesc	= { 0 };
	volumeDesc.Usage				= D3D11_USAGE_DEFAULT;
	volumeDesc.ByteWidth			= sizeof(VOXEL::TNode) * header.numNodes;
	volumeDesc.StructureByteStride	= sizeof(VOXEL::TNode);
	volumeDesc.BindFlags			= D3D11_BIND_UNORDERED_ACCESS;
	volumeDesc.MiscFlags			= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	volumeDesc.CPUAccessFlags		= 0;

	std::vector<VOXEL::TNode> nodes		= m_sparseOctree.GetNodes();
	D3D11_SUBRESOURCE_DATA volumeSRD	= { 0 };
	volumeSRD.pSysMem					= &nodes[0];

	hr = m_pDevice->CreateBuffer(&volumeDesc, &volumeSRD, &m_pRaycastNodeBuffer);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.ViewDimension		= D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements	= nodes.size();
	uavDesc.Format				= DXGI_FORMAT_UNKNOWN;

	hr = m_pDevice->CreateUnorderedAccessView(m_pRaycastNodeBuffer, &uavDesc, &m_pRaycastNodeUAV);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	nodes.clear();

	/* create shader voxel data */
	volumeDesc.ByteWidth			= sizeof(VOXEL::Voxel) * header.numData;
	volumeDesc.StructureByteStride	= sizeof(VOXEL::Voxel);

	std::vector<VOXEL::Voxel> voxels = m_sparseOctree.GetVoxels();
	volumeSRD.pSysMem = &voxels[0];

	hr = m_pDevice->CreateBuffer(&volumeDesc, &volumeSRD, &m_pRaycastVoxelBuffer);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	uavDesc.Buffer.NumElements = voxels.size();

	hr = m_pDevice->CreateUnorderedAccessView(m_pRaycastVoxelBuffer, &uavDesc, &m_pRaycastVoxelUAV);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	voxels.clear();

	/* create octree constant buffer */
	OctreeConstantBuffer ocb	= { 0 };
	ocb.gridLength				= header.gridlength;
	ocb.maxDepth				= header.numLevels;
	ocb.rootOrigin				= FVEC3(0, 0, 0);
	ocb.rootIndex				= header.numNodes - 1;

	D3D11_BUFFER_DESC ocbDesc	= { 0 };
	ocbDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	ocbDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	ocbDesc.ByteWidth			= sizeof(OctreeConstantBuffer);
	ocbDesc.Usage				= D3D11_USAGE_DYNAMIC;
	
	D3D11_SUBRESOURCE_DATA ocbSRD	= { 0 };
	ocbSRD.pSysMem					= &ocb;

	hr = m_pDevice->CreateBuffer(&ocbDesc, &ocbSRD, &m_pRaycastOctreeConstantBuffer);
	if (FAILED(hr))	{ PrintError(AT, hr); return hr; }

	//D3D11_MAPPED_SUBRESOURCE msr = { 0 };

	//hr = m_pContext->Map(m_pRaycastOctreeConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	//if (FAILED(hr))	{ PrintError(AT, hr); return hr; }

	//errno_t err = memcpy_s(msr.pData, msr.RowPitch, &ocb, sizeof(OctreeConstantBuffer));
	//m_pContext->Unmap(m_pRaycastOctreeConstantBuffer, 0);

	

	return S_OK;
}

HRESULT RasterRayApp::CreateDebugEnvironment(const signed _gridLength, const unsigned _unitSize)
{
	HRESULT hr = S_OK;

	/* --- Create grid lines --- */
	const unsigned gridLength = _gridLength;
	std::vector<FVEC3> gridLines;
	gridLines.resize(gridLength * gridLength * 8);
	std::vector<unsigned> gridIndices;
	gridIndices.resize(gridLength * gridLength * 8);

	FVEC3 lineOffset[8] {	FVEC3(0, 0, _unitSize), 
							FVEC3(_unitSize, 0, _unitSize), 
							FVEC3(_unitSize, 0, _unitSize), 
							FVEC3(_unitSize, 0, 0), 
							FVEC3(_unitSize, 0, 0), 
							FVEC3(0, 0, 0), 
							FVEC3(0, 0, 0), 
							FVEC3(0, 0, _unitSize) };
	unsigned index = 0;
	for (signed depth = 0; depth < gridLength; ++depth)
		for (signed width = 0; width < gridLength; ++width)
			for (unsigned k = 0; k < 8; ++k)
			{
				gridLines[index] = FVEC3(width * _unitSize, 0, depth * _unitSize) + lineOffset[k];
				gridIndices[index] = index;
				index++;
			}

	/* vertex buffer */
	D3D11_BUFFER_DESC gridDesc		= { 0 };
	gridDesc.Usage					= D3D11_USAGE_IMMUTABLE;
	gridDesc.BindFlags				= D3D11_BIND_VERTEX_BUFFER;
	gridDesc.CPUAccessFlags			= 0;
	gridDesc.StructureByteStride	= sizeof(FVEC3);
	gridDesc.ByteWidth				= sizeof(FVEC3) * gridLength * gridLength * 8;

	D3D11_SUBRESOURCE_DATA gridSRD	= { 0 };
	gridSRD.pSysMem					= &gridLines[0];

	hr = m_pDevice->CreateBuffer(&gridDesc, &gridSRD, &m_pDebugGridVertexBuffer);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/* index buffer */
	gridDesc						= { 0 };
	gridDesc.Usage					= D3D11_USAGE_IMMUTABLE;
	gridDesc.BindFlags				= D3D11_BIND_INDEX_BUFFER;
	gridDesc.StructureByteStride	= sizeof(unsigned);
	gridDesc.ByteWidth				= sizeof(unsigned) * gridLength * gridLength * 8;

	gridSRD			= { 0 };
	gridSRD.pSysMem = &gridIndices[0];

	hr = m_pDevice->CreateBuffer(&gridDesc, &gridSRD, &m_pDebugGridIndexBuffer);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	/* --- Debug Shader --- */
	ID3D10Blob* pShaderByteCode = nullptr;

	/* Vertex shader */
	hr = CompileShader(L"Revised/Shaders/Grid.hlsl", "vs_5_0", pShaderByteCode, "MainVS");
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreateVertexShader(pShaderByteCode->GetBufferPointer(), pShaderByteCode->GetBufferSize(), 0, &m_pDebugGridVertexShader);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	D3D11_INPUT_ELEMENT_DESC gridLayout = { 0 };
	gridLayout.SemanticName				= "POSITION";
	gridLayout.Format					= DXGI_FORMAT_R32G32B32_FLOAT;
	gridLayout.InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;

	hr = m_pDevice->CreateInputLayout(&gridLayout, 1, pShaderByteCode->GetBufferPointer(), pShaderByteCode->GetBufferSize(), &m_pDebugGridInputLayout);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	pShaderByteCode->Release();

	/* Pixel shader */
	hr = CompileShader(L"Revised/Shaders/Grid.hlsl", "ps_5_0", pShaderByteCode, "MainPS");
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	hr = m_pDevice->CreatePixelShader(pShaderByteCode->GetBufferPointer(), pShaderByteCode->GetBufferSize(), 0, &m_pDebugGridPixelShader);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	pShaderByteCode->Release();

	/* Constant buffer */
	D3D11_BUFFER_DESC dbgDesc	= { 0 };
	dbgDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	dbgDesc.ByteWidth			= sizeof(DebugConstantBuffer);
	dbgDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	dbgDesc.Usage				= D3D11_USAGE_DYNAMIC;

	hr = m_pDevice->CreateBuffer(&dbgDesc, 0, &m_pDebugGridConstantBuffer);
	if (FAILED(hr)) { PrintError(AT, hr); return hr; }

	return S_OK;
}