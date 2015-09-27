#include "Rasterizer.h"

Rasterizer::Rasterizer() :	m_pVertexShaderStage(nullptr),
							m_pPixelShaderStage(nullptr),
							m_pGeometryShaderStage(nullptr)
{ 
	ZeroMemory(&m_pDeferredRenderTargets, sizeof(ID3D11RenderTargetView) * RENDERTARGET_COUNT);
}

Rasterizer::~Rasterizer()
{
	SAFE_DELETE(m_pPixelShaderStage);
	SAFE_DELETE(m_pGeometryShaderStage);
	SAFE_DELETE(m_pVertexShaderStage);

	for (unsigned int i = 0; i < RENDERTARGET_COUNT; ++i)
		m_pDeferredRenderTargets[i] = nullptr;
}


bool Rasterizer::VInit(DXDriver* _driver)
{
	if (m_pVertexShaderStage || m_pPixelShaderStage || m_pGeometryShaderStage)
	{
		PrintError(AT, "shader stages already created");
		return false;
	}

	m_pVertexShaderStage	= VNEW VertexShader();
	m_pGeometryShaderStage	= VNEW GeometryShader();
	m_pPixelShaderStage		= VNEW PixelShader();

	if (!m_pVertexShaderStage->Init(L"Assets/Shaders/RasterNodeVS.hlsl", _driver->GetDevice()))
	//if (!m_pVertexShaderStage->Init(L"Assets/Shaders/vtest.hlsl", _driver->GetDevice()))
	{
		PrintError(AT, "failed to create vertex shader stage");
		return false;
	}

	if (!m_pGeometryShaderStage->Init(L"Assets/Shaders/RasterNodeGS.hlsl", _driver->GetDevice()))
	//if (!m_pGeometryShaderStage->Init(L"Assets/Shaders/gtest.hlsl", _driver->GetDevice()))
	{
		PrintError(AT, "failed to create geometry shader stage");
		return false;
	}

	if (!m_pPixelShaderStage->Init(L"Assets/Shaders/RasterNodePS.hlsl", _driver->GetDevice()))
	//if (!m_pPixelShaderStage->Init(L"Assets/Shaders/ptest.hlsl", _driver->GetDevice()))
	{
		PrintError(AT, "failed to create pixel shader stage");
		return false;
	}

	RenderTarget* renderTarget = nullptr;
	for (unsigned int i = RT_COLOR; i < RENDERTARGET_COUNT; ++i)
	{
		_driver->GetRenderTarget(i, renderTarget);
		if (!renderTarget)
			return false;

		renderTarget->GetView(m_pDeferredRenderTargets[i]);

		if (!m_pDeferredRenderTargets[i])
			return false;
	}

	return true;
}

void Rasterizer::VDraw(DXDriver* _driver, D3DBuffer* _model, D3DBuffer* _indices)
{
	_driver->SetZBufferOn();

	_driver->ClearRenderTargets(m_pDeferredRenderTargets, RENDERTARGET_COUNT);
	_driver->SetRenderTargets(m_pDeferredRenderTargets, RENDERTARGET_COUNT);

	m_pVertexShaderStage->VStage(_driver->GetContext());
	m_pGeometryShaderStage->VStage(_driver->GetContext());
	m_pPixelShaderStage->VStage(_driver->GetContext());

	uint32_t offset = 0;
	uint32_t stride = _model->GetSElement();

	ID3D11Buffer* vbuffers[] = { (ID3D11Buffer*)_model->GetResource() };

	_driver->GetContext()->IASetVertexBuffers(0, 1, vbuffers, &stride, &offset);
	_driver->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	
	if (_indices)
	{
		_driver->GetContext()->IASetIndexBuffer((ID3D11Buffer*)_indices->GetResource(), DXGI_FORMAT_R32_UINT, 0);
		_driver->GetContext()->DrawIndexed(_indices->GetNElements(), 0, 0);
	}
	else
	{
		_driver->GetContext()->Draw(_model->GetNElements(), 0);
	}

	ID3D11RenderTargetView* empties[RENDERTARGET_COUNT] = { NULL, NULL, NULL };
	_driver->SetRenderTargets(empties, RENDERTARGET_COUNT);
}