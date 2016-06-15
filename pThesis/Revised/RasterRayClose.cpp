#include "RasterRayApp.h"



template<typename T>
void SafeRelease(T*& _pInstance)
{
	if (_pInstance) _pInstance->Release();
	_pInstance = nullptr;
}

using RasterRay::RasterRayApp;

void RasterRayApp::ReleaseDXGI(void)
{
	m_displayMode = { 0 };
	m_adapterMemory = 0;
	m_adapterName = "";

	SafeRelease(m_pDxgiOutput);
	SafeRelease(m_pDxgiAdapter);
	SafeRelease(m_pDxgiFactory);
}

void RasterRayApp::ReleaseSwapChain(void)
{
	SafeRelease(m_pSwapChain);
}

void RasterRayApp::ReleaseDeviceAndContext(void)
{
	m_pContext->ClearState();
	m_pContext->Flush();

	SafeRelease(m_pContext);
#ifdef _DEBUG
	ID3D11Debug* pDebug;
	m_pDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&pDebug);

	SafeRelease(m_pDevice);

	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	pDebug->Release();
#else
	SafeRelease(m_pDevice);
#endif
	/* any object (not the device) that still reports a refcount > 1 is a problem, 
		- intrefs are not a problems since they're cleaned up with the release of the device itself */
}

void RasterRayApp::ReleaseBackBufferEnvironment(void)
{
	SafeRelease(m_pBackBufferIndexBuffer);
	SafeRelease(m_pBackBufferVertexBuffer);
	SafeRelease(m_pGlobalCameraConstantBuffer);
	SafeRelease(m_pBackBufferPixelShader);
	SafeRelease(m_pBackBufferInputLayout);
	SafeRelease(m_pBackBufferVertexShader);
	SafeRelease(m_pBlendState);
	SafeRelease(m_pPointSampler);
	SafeRelease(m_pDepthStencilView);
//	SafeRelease(m_pDepthStencilDisabled);
//	SafeRelease(m_pDepthStencilEnabled);
	SafeRelease(m_pDepthStencilTexture);
	SafeRelease(m_pBackBufferRenderTargetView);
	SafeRelease(m_pBackBufferTexture);
}

void RasterRayApp::ReleaseDeferredEnvironment(void)
{
	SafeRelease(m_pGlobalWindowConstantBuffer);
	
	SafeRelease(m_pDeferredUnorderedAccessViews[DRTI_Depth]);
	SafeRelease(m_pDeferredShaderResourceViews[DRTI_Depth]);
	SafeRelease(m_pDeferredRenderTargetViews[DRTI_Depth]);
	SafeRelease(m_pDeferredTextures[DRTI_Depth]);
	
	SafeRelease(m_pDeferredUnorderedAccessViews[DRTI_Normal]);
	SafeRelease(m_pDeferredShaderResourceViews[DRTI_Normal]);
	SafeRelease(m_pDeferredRenderTargetViews[DRTI_Normal]);
	SafeRelease(m_pDeferredTextures[DRTI_Normal]);

	SafeRelease(m_pDeferredUnorderedAccessViews[DRTI_Color]);
	SafeRelease(m_pDeferredShaderResourceViews[DRTI_Color]);
	SafeRelease(m_pDeferredRenderTargetViews[DRTI_Color]);
	SafeRelease(m_pDeferredTextures[DRTI_Color]);
}

void RasterRayApp::ReleaseRasterStates(void)
{
	//SafeRelease(m_pCurrentRasterizerState);

	SafeRelease(m_pRasterizerStates[RSI_CFFW].m_pState);
	SafeRelease(m_pRasterizerStates[RSI_CFFS].m_pState);
	SafeRelease(m_pRasterizerStates[RSI_CBFW].m_pState);
	SafeRelease(m_pRasterizerStates[RSI_CBFS].m_pState);
	SafeRelease(m_pRasterizerStates[RSI_CNFW].m_pState);
	SafeRelease(m_pRasterizerStates[RSI_CNFS].m_pState);
}

void RasterRayApp::ReleaseRasterizer(void)
{
	SafeRelease(m_pRasterizerPixelShader);
	SafeRelease(m_pRasterizerGeometryInputLayout);
	SafeRelease(m_pRasterizerVertexShader);
}

void RasterRayApp::ReleaseDebugEnvironment(void)
{
	SafeRelease(m_pDebugGridConstantBuffer);
	SafeRelease(m_pDebugGridPixelShader);
	SafeRelease(m_pDebugGridInputLayout);
	SafeRelease(m_pDebugGridVertexShader);
	SafeRelease(m_pDebugGridVertexBuffer);
}

void RasterRayApp::ReleaseRayCaster(void)
{
	SafeRelease(m_pRaycastOctreeConstantBuffer);
	SafeRelease(m_pRaycastVoxelUAV);
	SafeRelease(m_pRaycastVoxelBuffer);
	SafeRelease(m_pRaycastNodeUAV);
	SafeRelease(m_pRaycastNodeBuffer);
	SafeRelease(m_pRayCastComputeShader);
}