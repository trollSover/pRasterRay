#include "DebugRenderer.h"

DebugRenderer::DebugRenderer() :	m_pVertexShaderStage(0),
									m_pPixelShaderStage(0)									
{
	ZeroMemory(&m_pDeferredRenderTargets, sizeof(ID3D11RenderTargetView) * RENDERTARGET_COUNT);
}

DebugRenderer::~DebugRenderer()
{
	SAFE_DELETE(m_pPixelShaderStage);
	SAFE_DELETE(m_pVertexShaderStage);

	for (unsigned int i = 0; i < RENDERTARGET_COUNT; ++i)
		m_pDeferredRenderTargets[i] = nullptr;
}

bool DebugRenderer::VInit(DXDriver* _driver)
{
	if (m_pVertexShaderStage || m_pPixelShaderStage)
	{
		//PrintError(AT, "shader stages already created");
		return false;
	}

	m_pVertexShaderStage = new VertexShader();
	m_pPixelShaderStage = new PixelShader();

	if (!m_pVertexShaderStage->Init(L"Assets/Shaders/vertexDebug.hlsl", _driver->GetDevice()))
	{
		//PrintError(AT, "failed to create vertex shader stage");
		return false;
	}

	if (!m_pPixelShaderStage->Init(L"Assets/Shaders/pixelDebug.hlsl", _driver->GetDevice()))
	{
		//PrintError(AT, "failed to create pixel shader stage");
		return false;
	}

	RenderTarget* renderTarget = nullptr;
	_driver->GetRenderTarget(RT_COLOR, renderTarget);
	renderTarget->GetView(m_pDeferredRenderTargets[0]);
	_driver->GetRenderTarget(RT_NORMAL, renderTarget);
	renderTarget->GetView(m_pDeferredRenderTargets[1]);
	_driver->GetRenderTarget(RT_DEPTH, renderTarget);
	renderTarget->GetView(m_pDeferredRenderTargets[2]);

	return true;
}

void DebugRenderer::VDraw(DXDriver* _driver, D3DBuffer* _model, D3DBuffer* _indices)
{
	_driver->SetZBufferOn();

	_driver->ClearRenderTargets(m_pDeferredRenderTargets, RENDERTARGET_COUNT);
	_driver->SetRenderTargets(m_pDeferredRenderTargets, RENDERTARGET_COUNT);

	m_pVertexShaderStage->VStage(_driver->GetContext());
	m_pPixelShaderStage->VStage(_driver->GetContext());

	uint32_t offset = 0;
	uint32_t stride = _model->GetSElement();

	ID3D11Buffer* vbuffers[] = { (ID3D11Buffer*)_model->GetResource() };

	_driver->GetContext()->IASetVertexBuffers(0, 1, vbuffers, &stride, &offset);
	_driver->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_driver->GetContext()->Draw(_model->GetNElements(), 0);

	ID3D11RenderTargetView* empties[RENDERTARGET_COUNT] = { NULL, NULL, NULL };

	_driver->SetRenderTargets(empties, RENDERTARGET_COUNT);
}