#pragma once

#include "CoreStd.h"

D3D11_TEXTURE2D_DESC GetDepthBufferDesc(Resolution _res)
{
	D3D11_TEXTURE2D_DESC desc;

	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.Width				= _res.width;
	desc.Height				= _res.height;
	desc.MipLevels			= 1;
	desc.ArraySize			= 1;
	desc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count	= 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage				= D3D11_USAGE_DEFAULT;
	desc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags		= 0;
	desc.MiscFlags			= 0;
	

	return desc;
}

D3D11_DEPTH_STENCIL_DESC GetDepthStencilDesc()
{
	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	// Set up the description of the stencil state.
	desc.DepthEnable	= true;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.DepthFunc		= D3D11_COMPARISON_LESS;

	desc.StencilEnable		= true;
	desc.StencilReadMask	= 0xFF;
	desc.StencilWriteMask	= 0xFF;

	// Stencil operations if pixel is front-facing.
	desc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_INCR;
	desc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	desc.BackFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_DECR;
	desc.BackFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
	desc.BackFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
	
	return desc;
}

D3D11_DEPTH_STENCIL_VIEW_DESC GetDepthStencilViewDesc()
{
	D3D11_DEPTH_STENCIL_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

	desc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipSlice = 0;
	
	return desc;
}

DXGI_SWAP_CHAIN_DESC GetSwapChainDesc(Resolution _res, bool _windowed, HWND _hWnd)
{
	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));

	desc.BufferCount		= 1;
	desc.BufferDesc.Width	= _res.width;
	desc.BufferDesc.Height	= _res.height;
	desc.BufferDesc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;

	desc.BufferDesc.RefreshRate.Numerator	= 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;

	desc.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.OutputWindow		= _hWnd;
	desc.SampleDesc.Count	= 1;
	desc.SampleDesc.Quality = 0;
	desc.Windowed			= _windowed;
	
	desc.BufferDesc.ScanlineOrdering	= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling				= DXGI_MODE_SCALING_UNSPECIFIED;

	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	desc.Flags		= 0;

	return desc;
}

D3D11_RASTERIZER_DESC GetRasterizerDesc(D3D11_CULL_MODE _cullMode = D3D11_CULL_NONE, D3D11_FILL_MODE _fillMode = D3D11_FILL_SOLID)
{
	D3D11_RASTERIZER_DESC desc;

	desc.AntialiasedLineEnable = false;

	desc.CullMode				= _cullMode; // D3D11_CULL_BACK; //     D3D11_CULL_FRONT; // 
	desc.DepthBias				= 0;
	desc.DepthBiasClamp			= 0.0f;
	desc.DepthClipEnable		= true; // false; // 
	desc.FillMode				= _fillMode; // D3D11_FILL_WIREFRAME;// 
	desc.FrontCounterClockwise	= false;
	desc.MultisampleEnable		= false;
	desc.ScissorEnable			= false;
	desc.SlopeScaledDepthBias	= 0.0f;

	return desc;
}

D3D11_BLEND_DESC GetBlendDesc(unsigned int _renderTargetCount)
{
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable					= FALSE;
	blendDesc.IndependentBlendEnable				= FALSE;

	for (unsigned int i = 0; i < _renderTargetCount; ++i)
	{
		blendDesc.RenderTarget[i].BlendEnable			= FALSE;
		blendDesc.RenderTarget[i].SrcBlend				= D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlend				= D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].BlendOp				= D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha			= D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha		= D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOpAlpha			= D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = 0x0f;
	}
	return blendDesc;

}

bool QueryDXGI(Resolution _res, int& _videoCardMemory, char*& _videoCardDescription)
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator, stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;

	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	// Use the factory to create an adapter for the primary graphics interface (video card).
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	// Enumerate the primary adapter output (monitor).
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.

	for (i = 0; i<numModes; i++)
	{
		if (displayModeList[i].Width == _res.width)
		{
			if (displayModeList[i].Height == _res.height)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes.
	_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
#ifdef _WIN64
	error = 0;
#else
	error = wcstombs_s(&stringLength, _videoCardDescription, 128, adapterDesc.Description, 128);
#endif
	if (error != 0)
	{
		return false;
	}

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = 0;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter.
	adapter->Release();
	adapter = 0;

	// Release the factory.
	factory->Release();
	factory = 0;

	return true;
}

