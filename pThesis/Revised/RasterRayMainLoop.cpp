#include "RasterRayApp.h"
#include "../Global/TimerQPC.h"

using namespace RasterRay;

LRESULT	RasterRayApp::VApplicationProc(HWND _hwnd, unsigned _msg, WPARAM _wparam, LPARAM _lparam)
{
	switch (_msg)
	{
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
		break;
	case WM_INPUT:
		m_pInput->Update(_hwnd, (HRAWINPUT&)_lparam);
		break;
	default:
		break;
	}
	return DefWindowProc(_hwnd, _msg, _wparam, _lparam);
}

void RasterRayApp::RunApplication(IApplication* _pApplication)
{
	MSG msg;
	bool run = true;
	ZeroMemory(&msg, sizeof(MSG));
	ITimer* pTimer = VNEW TimerQPC();
	pTimer->VInit();

	HRESULT frameOk;
	while (run)
	{
		pTimer->VUpdate();

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				run = false;
			}
		}

		Time dt	= pTimer->VGetTime();			/* get time delta */
		frameOk = _pApplication->VFrame(dt);	/* update application */

		if (FAILED(frameOk))
		{
			run = false;
			PrintError(AT, frameOk);
		}
	}

	delete pTimer;
	// dont delete iapplication here, do it afterwards
}

HRESULT RasterRayApp::VFrame(Time _time)
{
	HRESULT hr = S_OK;

	if (IsKeyPress(VK_ESCAPE))
	{
		PostQuitMessage(0);
		return true;
	}

	/* Update Camera */
	UpdateCamera(_time);

	/* Update States */
	UpdateRenderState();

	ReloadShader("hello");

	/* Update UI */
	UpdateUI(_time);

	/* Update Global Buffers */
	UpdateGlobalShaderBuffers();

	// Begin draw
	const float bgcolor[]		= { 0.f, 0.f, 0.f, 0.f };
	const float blendfactor[]	= { 0.f, 0.f, 0.f, 0.f };

	m_pContext->ClearRenderTargetView(m_pBackBufferRenderTargetView, bgcolor);
	m_pContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.f, 0);

	//m_pContext->OMSetDepthStencilState(m_pCurrentDepthStencilState, 1);
	ResetDepthMode();
	ResetRasterState();
	m_pContext->OMSetBlendState(m_pBlendState, blendfactor, 0xffffffff);
	//m_pContext->RSSetState(m_pRasterState);

	/* Set up render scene */
	// note: broken - need to find what changed

	//m_frustum.Build(1000, m_pCamera->GetProjectionMatrix(), m_pCamera->GetViewMatrix());
	//m_raycastSceneCB = m_frustum.SetRaycastScene(m_sparseOctree.GetNodes(), m_sparseOctree.GetHeader());

	//D3D11_SUBRESOURCE_DATA ocbSRD = { 0 };
	//ocbSRD.pSysMem = &m_raycastSceneCB;

	//D3D11_BUFFER_DESC ocbDesc = { 0 };
	//ocbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//ocbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//ocbDesc.ByteWidth = sizeof(OctreeConstantBuffer);
	//ocbDesc.Usage = D3D11_USAGE_DYNAMIC;

	//hr = m_pDevice->CreateBuffer(&ocbDesc, &ocbSRD, &m_pRaycastOctreeConstantBuffer);
	//if (FAILED(hr))	{ PrintError(AT, hr); return hr; }


	// draw ...
	if (m_drawDebug)
		DrawDebug();

	if (m_drawRasterized)
		DrawRasterized();

	if (m_drawRayCast)
		DrawRayCast();

	//DrawRasterized();

	// draw UI
	DrawUI(_time);

	// draw deferred to backbuffer
	m_pContext->RSSetState(m_pRasterizerStates[RSI_CBFS].m_pState);
	SetDepthMode(DSSI_DISABLED);
	DrawToBackBuffer();

	// End draw
	for (unsigned i = 0; i < RENDERTARGET_COUNT; ++i)
		m_pContext->ClearRenderTargetView(m_pDeferredRenderTargetViews[i], bgcolor);

	hr = m_pSwapChain->Present(0, 0);

	return hr;
}