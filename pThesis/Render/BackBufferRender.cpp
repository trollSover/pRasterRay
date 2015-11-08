#include "BackBufferRender.h"

BackBufferRender::BackBufferRender() :	m_pBackBufferShaderVS(nullptr),
										m_pBackBufferShaderPS(nullptr),
										m_pPointSampler(nullptr)
{
	for (int i = 0; i < RENDERTARGET_COUNT; ++i)
		m_pShaderResources[i] = nullptr;
}

BackBufferRender::~BackBufferRender()
{
	SAFE_DELETE(m_pBackBufferShaderPS);
	SAFE_DELETE(m_pBackBufferShaderVS);
	
	/* since we're only dealing with references we just null our end and let the host properly release/delete them */
	for (unsigned int i = 0; i < RT_COLOR + RENDERTARGET_COUNT; ++i)
		m_pShaderResources[i] = nullptr;

	SAFE_RELEASE(m_pPointSampler);

	m_pBackBufferRTV	= nullptr;
}

bool BackBufferRender::FetchShaderResource(const unsigned int _Id, DXDriver* _driver, ID3D11ShaderResourceView*& _resource)
{
	RenderTarget* refTarget = nullptr;
	_driver->GetRenderTarget(_Id, refTarget);

	// the rendertarget does not exist
	if (!refTarget)
	{
		PrintError(AT, std::to_string(_Id) + " target does not exist");
		return false;
	}

	refTarget->GetShaderResource(_resource);

	if (!_resource)
	{
		PrintError(AT, std::to_string(_Id) + " target does not provide itself as a shader resource");
		return false;
	}

	return true;
}

bool BackBufferRender::VInit(DXDriver* _driver)
{
	// construct & initialize final deferred rendering stages
	if (m_pBackBufferShaderVS || m_pBackBufferShaderPS)
	{
		PrintError(AT, "shader stages already created");
		return false;
	}

	m_pBackBufferShaderVS = VNEW VertexShader();
	m_pBackBufferShaderPS = VNEW PixelShader();

	if (!m_pBackBufferShaderVS->Init(L"Assets/FinalOutputVS.hlsl", _driver->GetDevice()))
	{
		PrintError(AT, "failed to create vertex shader stage");
		return false;
	}

	if (!m_pBackBufferShaderPS->Init(L"Assets/FinalOutputPS.hlsl", _driver->GetDevice()))
	{
		PrintError(AT, "failed to create pixel shader stage");
		return false;
	}
	
	// fetch shader resource references needed
	RenderTarget* renderTarget = nullptr;
	_driver->GetRenderTarget(RT_COLOR, renderTarget);
	renderTarget->GetShaderResource(m_pShaderResources[0]);
	_driver->GetRenderTarget(RT_NORMAL, renderTarget);
	renderTarget->GetShaderResource(m_pShaderResources[1]);
	_driver->GetRenderTarget(RT_DEPTH, renderTarget);
	renderTarget->GetShaderResource(m_pShaderResources[2]);

	// fetch backbuffer rendertarget reference
	RenderTarget* temp = _driver->GetBackBuffer();
	temp->GetView(m_pBackBufferRTV);

	if (!m_pBackBufferRTV)
	{
		PrintError(AT, "failed to fetch back buffer");
		return false;
	}

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU		= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV		= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW		= D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias		= 0.0f;
	samplerDesc.MaxAnisotropy	= 1;
	samplerDesc.ComparisonFunc	= D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0]	= 0;
	samplerDesc.BorderColor[1]	= 0;
	samplerDesc.BorderColor[2]	= 0;
	samplerDesc.BorderColor[3]	= 0;
	samplerDesc.MinLOD			= 0;
	samplerDesc.MaxLOD			= D3D11_FLOAT32_MAX;

	HRESULT result = _driver->GetDevice()->CreateSamplerState(&samplerDesc, &m_pPointSampler);
	if (FAILED(result))
	{
		PrintError(AT, result);
		return false;
	}

	return true;
}

void BackBufferRender::VDraw(DXDriver* _driver, D3DBuffer* _model, D3DBuffer* _indices)
{
	_driver->SetRasterizerState(D3D11_CULL_BACK, D3D11_FILL_SOLID);

	ID3D11DeviceContext* context = _driver->GetContext();

	// turn off depth & set rendertarget to backbuffer
	_driver->SetZBufferOff();
	_driver->SetRenderTargets(&m_pBackBufferRTV, 1);

	// set final deferred stage shaders & resources
	m_pBackBufferShaderVS->VStage(context);
	m_pBackBufferShaderPS->VStage(context);

	context->PSSetShaderResources(3, RENDERTARGET_COUNT, m_pShaderResources);
	context->PSSetSamplers(0, 1, &m_pPointSampler);

	// draw
	context->Draw(3, 0);
	
	// reset to former state
	ID3D11ShaderResourceView* empties[RENDERTARGET_COUNT] = { NULL, NULL, NULL };
	context->PSSetShaderResources(3, RENDERTARGET_COUNT, empties);

	//ID3D11RenderTargetView* empty = nullptr;
	//_driver->SetRenderTargets(&empty, 1);
	//_driver->SetZBufferOn();
}

