#include "Overlay.h"


Overlay::Overlay(void) : m_pVertexShaderStage(nullptr), m_pPixelShaderStage(nullptr), m_pTexture(nullptr)
{ }

Overlay::~Overlay(void)
{
	
}

bool Overlay::VInit(DXDriver* _driver)
{
	if (m_pVertexShaderStage || m_pPixelShaderStage)
	{
		PrintError(AT, "shader stages already created");
		return false;
	}

	m_pVertexShaderStage	= VNEW VertexShader();
	m_pPixelShaderStage		= VNEW PixelShader();

	if (!m_pVertexShaderStage->Init(L"Assets/Shaders/overlayv.hlsl", _driver->GetDevice()))
	{
		PrintError(AT, "failed to create vertex shader stage");
		return false;
	}

	if (!m_pPixelShaderStage->Init(L"Assets/Shaders/overlayp.hlsl", _driver->GetDevice()))
	{
		PrintError(AT, "failed to create pixel shader stage");
		return false;
	}

	HRESULT hr = CoInitialize(NULL);
	m_pTexture = VNEW D3DBuffer();

	hr = CreateWICTextureFromFile (	_driver->GetDevice(), _driver->GetContext(), L"Assets/Fonts/font.png", 
									(ID3D11Resource**)m_pTexture->GetResourceRef(), (ID3D11ShaderResourceView**)m_pTexture->GetSRVRef(), 
									NULL);
	
	if (FAILED(hr))
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		PrintError(AT, errMsg);
		return false;
	}

	return true;
}

void Overlay::VDraw(DXDriver* _driver, D3DBuffer* _model)
{
	m_pVertexShaderStage->VStage(_driver->GetContext());
	m_pPixelShaderStage->VStage(_driver->GetContext());

	uint32_t offset = 0;
	uint32_t stride = _model->GetSElement();

	ID3D11ShaderResourceView* srvs[] = { m_pTexture->GetSRV() };

	ID3D11Buffer* vbuffers[] = { (ID3D11Buffer*)_model->GetResource() };

	_driver->GetContext()->PSSetShaderResources(0, 1, srvs);

	_driver->GetContext()->IASetVertexBuffers(0, 1, vbuffers, &stride, &offset);
	_driver->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_driver->GetContext()->Draw(_model->GetNElements(), 0);
}