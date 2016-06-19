#include "RasterRayApp.h"

using namespace RasterRay;

template<typename T>
void SafeDelete(T*& _pInstance)
{
	if (_pInstance) delete _pInstance;
	_pInstance = nullptr;
}



RasterRayApp::RasterRayApp(void) :		m_hwnd(0),
										m_hInstance(0),
										m_pContext(nullptr),
										m_pDevice(nullptr),
										m_pDxgiAdapter(nullptr),
										m_pDxgiFactory(nullptr),
										m_pDxgiOutput(nullptr),
										m_pSwapChain(nullptr),
										m_pBackBufferTexture(nullptr),
										m_adapterMemory(0),
										m_adapterName(""),
										m_pInput(nullptr),
										m_pFontWrapper(nullptr),
										m_pCamera(nullptr),
										m_pRasterState(nullptr),
										m_drawRasterized(true),
										m_drawRayCast(false),
										m_drawDebug(true),
										m_drawUI(true),
										m_drawUIHelp(false),
										m_fillMode(D3D11_FILL_SOLID),
										m_cullMode(D3D11_CULL_BACK),
										m_FPS(0),
										//m_pDepthStencilDisabled(nullptr),
										//m_pDepthStencilEnabled(nullptr),
										m_pDepthStencilTexture(nullptr),
										m_pDepthStencilView(nullptr),
										m_pBlendState(nullptr),
										m_lodLevel(0),
										m_currentDepthStateIndex(0),
										m_currentRasterStateIndex(0),
										m_pBackBufferIndexBuffer(0),
										m_pBackBufferInputLayout(0),
										m_pBackBufferPixelShader(0),
										m_pBackBufferRenderTargetView(0),
										m_pBackBufferVertexBuffer(0),
										m_pBackBufferVertexShader(0),
										m_pDebugBoundingVolumeBuffer(0),
										m_pDebugBoundingVolumeIndices(0),
										m_pDebugGridConstantBuffer(0),
										m_pDebugGridIndexBuffer(0),
										m_pDebugGridInputLayout(0),
										m_pDebugGridPixelShader(0),
										m_pDebugGridVertexBuffer(0),
										m_pDebugGridVertexShader(0),
										m_pGlobalCameraConstantBuffer(0),
										m_pGlobalWindowConstantBuffer(0),
										m_pPointSampler(0),
										m_pRasterizerGeometryInputLayout(0),
										m_pRasterizerPixelShader(0),
										m_pRasterizerVertexShader(0),
										m_pRayCastComputeShader(0),
										m_pRaycastNodeBuffer(0),
										m_pRaycastNodeUAV(0),
										m_pRaycastOctreeConstantBuffer(0),
										m_pRaycastVoxelBuffer(0),
										m_pRaycastVoxelUAV(0),
										m_resWidth(800),
										m_resHeight(600),
										m_dispatchX(1),
										m_dispatchY(1)


{	
	for (int i = 0; i < 3; ++i)
	{
		m_pDeferredRenderTargetViews[i] = nullptr;
		m_pDeferredShaderResourceViews[i] = nullptr;
		m_pDeferredTextures[i] = nullptr;
		m_pDeferredUnorderedAccessViews[i] = nullptr;
	}
}

RasterRayApp::~RasterRayApp(void)
{	
	/* Release all resource in reverse order of initialization */
	
	// this is ridicolous - some COM objects refuse to die 
	if (m_pContext)
	{
		m_pContext->ClearState();
		m_pContext->Flush();

		m_pContext->RSSetState(nullptr);
		m_pContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		m_pContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		m_pContext->IASetInputLayout(nullptr);
		m_pContext->OMSetBlendState(nullptr, 0, 0);
		m_pContext->OMSetDepthStencilState(nullptr, 0);
		m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
	}
	//if (m_pCurrentDepthStencilState) m_pCurrentDepthStencilState->Release();
	//m_pCurrentDepthStencilState = nullptr;
	//if (m_pCurrentRasterizerState) m_pCurrentRasterizerState->Release();
	//m_pCurrentRasterizerState = nullptr;

	ReleaseRayCaster();
	ReleaseDebugEnvironment();

	if (m_pFontWrapper) m_pFontWrapper->Release();
	m_pFontWrapper = nullptr;

	SafeDelete(m_pCamera);
	SafeDelete(m_pInput);

	ReleaseRasterizer();
	ReleaseDeferredEnvironment();
	ReleaseBackBufferEnvironment();
	ReleaseSwapChain();

	if (m_pDevice)
	{
		ReleaseDeviceAndContext();
	}

	DestroyWindow(m_hwnd);
	m_hwnd = nullptr;
	UnregisterClass("RasterRay", m_hInstance);
	m_hInstance = nullptr;

	//-------
}

