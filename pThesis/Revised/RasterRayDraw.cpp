#include "RasterRayApp.h"
#include <sstream>

using namespace RasterRay;

void RasterRayApp::DrawDebug(void)
{
	const float Rad90 = 90.f * 3.1415f / 180.f;
	const float CosRad90 = cos(Rad90);
	const float SinRad90 = sin(Rad90);
	const float axisX[16] = { 
		1, 0, 0, 0, 
		0, CosRad90, -SinRad90, m_debugGridLength * m_debugGridUnitSize,
		0, SinRad90, CosRad90, m_debugGridLength * m_debugGridUnitSize,
		0, 0, 0, 1 
	};

	const float axisY[16] = { 
		CosRad90, 0, SinRad90, 0, 
		0, 1, 0, 0, 
		-SinRad90, 0, CosRad90, m_debugGridLength * m_debugGridUnitSize,
		0, 0, 0, 1 
	};

	const float axisZ[16] = { 
		CosRad90, -SinRad90, 0, m_debugGridLength * m_debugGridUnitSize,
		SinRad90, CosRad90, 0, 0, 
		0, 0, 1, 0, 
		0, 0, 0, 1 
	};

	const FVEC4 color[] = { FVEC4(1, 0, 0, 1), FVEC4(0, 1, 0, 1), FVEC4(0, 0, 1, 1) };

	FMAT4X4 rotationPlanes[3] = { axisX, axisY, axisZ };

	D3D11_MAPPED_SUBRESOURCE msr = { 0 };
	DebugConstantBuffer dbgcb;
	

	//m_pContext->ClearState();
	m_pContext->RSSetViewports(1, &m_viewPort);

	//m_pContext->RSSetState(m_pRasterizerStates[RSI_CBFS]);
	//m_pContext->OMSetDepthStencilState(m_pDepthStencilDisabled, 1);
	SetDepthMode(DSSI_DISABLED);

	ID3D11RenderTargetView* views[RENDERTARGET_COUNT] = { m_pDeferredRenderTargetViews[DRTI_Color], m_pDeferredRenderTargetViews[DRTI_Normal], m_pDeferredRenderTargetViews[DRTI_Depth] };
	m_pContext->OMSetRenderTargets(RENDERTARGET_COUNT, views, m_pDepthStencilView);

	/* Draw debug grid */

	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	unsigned stride = sizeof(FVEC3);
	unsigned offset = 0;
	m_pContext->IASetInputLayout(m_pDebugGridInputLayout);
	m_pContext->IASetVertexBuffers(0, 1, &m_pDebugGridVertexBuffer, &stride, &offset);
	m_pContext->IASetIndexBuffer(m_pDebugGridIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pContext->VSSetShader(m_pDebugGridVertexShader, 0, 0);
	m_pContext->PSSetShader(m_pDebugGridPixelShader, 0, 0);
	//m_pContext->Draw(256 * 256 * 8, 0);
	
	
	for (unsigned i = 0; i < 1; ++i)
	{
		
		dbgcb.rotateOnAxis = rotationPlanes[i];
		dbgcb.color = color[i];

		msr.pData = &dbgcb;

		HRESULT hr = m_pContext->Map(m_pDebugGridConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		if (FAILED(hr))	{ PrintError(AT, hr); continue; }
		unsigned size = sizeof(DebugConstantBuffer);
		errno_t err = memcpy_s(msr.pData, msr.RowPitch, &dbgcb, sizeof(DebugConstantBuffer));
		m_pContext->Unmap(m_pDebugGridConstantBuffer, 0);

		m_pContext->VSSetConstantBuffers(3, 1, &m_pDebugGridConstantBuffer);
		m_pContext->PSSetConstantBuffers(3, 1, &m_pDebugGridConstantBuffer);

		m_pContext->DrawIndexed(m_debugGridLength * m_debugGridLength * 8, 0, 0);
	}

	dbgcb.color = FVEC4(1,1,1,1);

	msr.pData = &dbgcb;

	HRESULT hr = m_pContext->Map(m_pDebugGridConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	if (FAILED(hr))	{ PrintError(AT, hr); return; }
	unsigned size = sizeof(DebugConstantBuffer);
	errno_t err = memcpy_s(msr.pData, msr.RowPitch, &dbgcb, sizeof(DebugConstantBuffer));
	m_pContext->Unmap(m_pDebugGridConstantBuffer, 0);

	m_pContext->VSSetConstantBuffers(3, 1, &m_pDebugGridConstantBuffer);
	m_pContext->PSSetConstantBuffers(3, 1, &m_pDebugGridConstantBuffer);

	for (unsigned i = 0; i <= m_lodLevel; ++i)
	{
		m_pContext->IASetVertexBuffers(0, 1, &m_pDebugBoundingVolumeBuffer[i], &stride, &offset);
		m_pContext->IASetIndexBuffer(m_pDebugBoundingVolumeIndices[i], DXGI_FORMAT_R32_UINT, 0);
		m_pContext->DrawIndexed(m_sparseOctree.m_boundingGridIndices[i].size(), 0, 0);
	}
	//m_pContext->ClearState();
	//m_pContext->RSSetState(m_pRasterizerStates[RSI_CBFS]);
	//m_pContext->RSSetViewports(1, &m_viewPort);
}
void RasterRayApp::DrawRasterized(void)
{
	ID3D11RenderTargetView* views[RENDERTARGET_COUNT] = { m_pDeferredRenderTargetViews[DRTI_Color], m_pDeferredRenderTargetViews[DRTI_Normal], m_pDeferredRenderTargetViews[DRTI_Depth] };
	m_pContext->OMSetRenderTargets(RENDERTARGET_COUNT, views, m_pDepthStencilView);

	m_currentDepthStateIndex = SetDepthMode(DSSI_ENABLED);
	m_pContext->RSSetState(m_pRasterizerStates[m_currentRasterStateIndex].m_pState);

	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->IASetInputLayout(m_pRasterizerGeometryInputLayout);
	unsigned stride = sizeof(Vertex);
	unsigned offset = 0;
	m_pContext->IASetVertexBuffers(0, 1, &m_pRasterizerGeometryVertexBuffers[m_lodLevel], &stride, &offset);

	m_pContext->VSSetShader(m_pRasterizerVertexShader, 0, 0);
	m_pContext->PSSetShader(m_pRasterizerPixelShader, 0, 0);

	m_pContext->Draw(m_sparseOctree.GetVertexCountAtLodDepth(m_lodLevel), 0);
	//m_pContext->DrawIndexed(4, 0, 0);

	m_pContext->ClearState();
	m_pContext->RSSetState(m_pRasterizerStates[RSI_CBFS].m_pState);
	m_pContext->RSSetViewports(1, &m_viewPort);
}

void RasterRayApp::DrawRayCast(void)
{
	m_pContext->ClearState();
	ID3D11RenderTargetView* rtvs[] = { nullptr, nullptr, nullptr };
	m_pContext->OMSetRenderTargets(3, rtvs, m_pDepthStencilView);

	m_pContext->RSSetViewports(1, &m_viewPort);
	m_pContext->CSSetConstantBuffers(1, 1, &m_pGlobalWindowConstantBuffer);
	m_pContext->CSSetConstantBuffers(2, 1, &m_pRaycastOctreeConstantBuffer);
	m_pContext->CSSetConstantBuffers(0, 1, &m_pGlobalCameraConstantBuffer);

	ID3D11UnorderedAccessView* uavs[5] = {	m_pRaycastNodeUAV, 
											m_pRaycastVoxelUAV,  
											m_pDeferredUnorderedAccessViews[DRTI_Depth], 
											m_pDeferredUnorderedAccessViews[DRTI_Color],
											m_pDeferredUnorderedAccessViews[DRTI_Normal]
										};

	m_pContext->CSSetUnorderedAccessViews(0, 5, uavs, 0);
	m_pContext->CSSetShader(m_pRayCastComputeShader, 0, 0);
	m_pContext->Dispatch(m_dispatchX, m_dispatchY, 1);
	m_pContext->CSSetShader(0, 0, 0);

	uavs[0] = nullptr;
	uavs[1] = nullptr;
	uavs[2] = nullptr;
	uavs[3] = nullptr;
	uavs[4] = nullptr;
	m_pContext->CSSetUnorderedAccessViews(0, 5, uavs, 0);


	rtvs[0] = m_pDeferredRenderTargetViews[DRTI_Color];
	rtvs[1] = m_pDeferredRenderTargetViews[DRTI_Normal];
	rtvs[2] = m_pDeferredRenderTargetViews[DRTI_Depth];

	m_pContext->OMSetRenderTargets(3, rtvs, m_pDepthStencilView);
}

void RasterRayApp::DrawUI(const Time& _time)
{
	SetDepthMode(DSSI_DISABLED);

	m_pContext->OMSetRenderTargets(1, &m_pDeferredRenderTargetViews[DRTI_Color], 0);

	FVEC3 cpos = m_pCamera->GetPosition();
	FVEC3 cdir = m_pCamera->GetEyeDir();

	const float fontSize = 16.0f;
	const float indx = 10.0f;
	float indy = 10.0f;

	std::wostringstream stream;
	stream.precision(5);
	std::wstring str;
	stream << "Camera::Position\nx: " << cpos.x << "\ny: " << cpos.y << "\nz: " << cpos.z << "\n" <<
		"Camera::Direction\nx: " << cdir.x << "\ny: " << cdir.y << "\nz: " << cdir.z <<
		"\nCamera::Speed: " << m_pCamera->GetMovementSpeed();

	str = stream.str();

	m_pFontWrapper->DrawString(m_pContext, str.c_str(), fontSize, indx, 10, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);
	stream.str(std::wstring());
	stream.flush();

	stream << "FPS = " << m_FPS << "\ndt(ms) = " << _time.dtMS;
	str = stream.str();

	m_pFontWrapper->DrawString(m_pContext, str.c_str(), fontSize, indx, 180, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);
	stream.str(std::wstring());
	stream.flush();

	std::string cullmode, fillmode;
	cullmode = m_pRasterizerStates[m_currentRasterStateIndex].cull;
	fillmode = m_pRasterizerStates[m_currentRasterStateIndex].fill;

	stream << "Cull Mode: " << cullmode.c_str() <<
		"\nFill Mode: " << fillmode.c_str() <<
		"\nRasterizer: " << (m_drawRasterized ? "on" : "off") <<
		"\nRayCaster: " << (m_drawRayCast ? "on" : "off") <<
		"\nDebug: " << (m_drawDebug ? "on" : "off");

	str = stream.str();

	m_pFontWrapper->DrawString(m_pContext, str.c_str(), fontSize, 200, 10, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);
	stream.str(std::wstring());
	stream.flush();

	stream << "Total Nodes = " << m_sparseOctree.GetHeader().numNodes << "\nVisible Root = " << m_raycastSceneCB.rootIndex << "\nRoot Origin = "
		<< m_raycastSceneCB.rootOrigin.x << ":" << m_raycastSceneCB.rootOrigin.y << ":" << m_raycastSceneCB.rootOrigin.z << "\nNode Size = " << m_raycastSceneCB.gridLength
		<< "\nRoot Depth = " << m_raycastSceneCB.maxDepth;

	str = stream.str();
	m_pFontWrapper->DrawString(m_pContext, str.c_str(), fontSize, indx, 220, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);

	stream.str(std::wstring());
	stream.flush();

	const WCHAR* help = L"Press 'p' for control mappings";
	m_pFontWrapper->DrawString(m_pContext, help, fontSize, VGetResolution().width - 260, 10, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);

	if (m_drawUIHelp)
	{
		const WCHAR* help = L"Move: WASD\nFree Look: Mouse (while left key down)\nToggle Cull: F1\nToggle Fill: F2\nCamera Speed: Numpad +/-\nToggle Rasterize: 1\nToggle RayCast: 2\nToggle Debug: 3\nRasterizer LoD: F5+/F6-\nYaw: T+/Y-\nRoll: G+/H-\nPitch: B+/N-\nCamera Reset: Home\nReload Raycast Shader: F8";
		m_pFontWrapper->DrawString(m_pContext, help, fontSize, VGetResolution().width - 260, 10 + fontSize + 3, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);
	}

	// position debug
	FMAT4X4 mwvp		= TMatrixTranspose(m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix());
	FMAT4X4	mwvpi		= XMMatrixInverse(&XMMatrixDeterminant(mwvp), mwvp);
	FMAT4X4 mwv			= TMatrixTranspose(m_pCamera->GetViewMatrix());
	FMAT4X4	mwvi		= XMMatrixInverse(&XMMatrixDeterminant(mwv), mwv);

	FVEC3 wvpp = FVEC3(mwvp.m03, mwvp.m13, mwvp.m23);
	FVEC3 wvppi = FVEC3(mwvpi.m03, mwvpi.m13, mwvpi.m23);
	FVEC3 wvp = FVEC3(mwv.m03, mwv.m13, mwv.m23);	
	FVEC3 wvpi = FVEC3(mwvi.m03, mwvi.m13, mwvi.m23);

	stream.str(std::wstring());
	stream.flush();

	stream << "wwpp = {" << wvpp.x << ":" << wvpp.y << ":" << wvpp.z << "}\n"
		<< "wvppi = {" << wvppi.x << ":" << wvppi.y << ":" << wvppi.z << "}\n"
		<< "wvp = {" << wvp.x << ":" << wvp.y << ":" << wvp.z << "}\n"
		<< "wvpi = {" << wvpi.x << ":" << wvpi.y << ":" << wvpi.z << "}\n";

	str = stream.str();
	m_pFontWrapper->DrawString(m_pContext, str.c_str(), fontSize, indx, 320, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);

	stream.str(std::wstring());
	stream.flush();

	stream << "Rasterizer LoD = " << m_lodLevel << " /" << m_sparseOctree.GetHeader().numLevels << "\nVertex Count = " << m_sparseOctree.GetVertexCountAtLodDepth(m_lodLevel);

	str = stream.str();
	m_pFontWrapper->DrawString(m_pContext, str.c_str(), fontSize, indx, 400, 0xff505050, FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE | FW1_ALIASED);

	stream.str(std::wstring());
	stream.flush();

	m_pContext->OMSetRenderTargets(0, 0, 0);
	m_pContext->OMSetDepthStencilState(0, 0);
}

void RasterRayApp::DrawToBackBuffer(void)
{
	// set
	SetDepthMode(DSSI_DISABLED);
	
	//m_pContext->OMSetDepthStencilState(m_pDepthStencilDisabled, 1);
	m_pContext->RSSetState(m_pRasterizerStates[RSI_CBFS].m_pState);
	m_pContext->OMSetRenderTargets(1, &m_pBackBufferRenderTargetView, m_pDepthStencilView);
	m_pContext->IASetInputLayout(m_pBackBufferInputLayout);
	m_pContext->VSSetShader(m_pBackBufferVertexShader, 0, 0);
	m_pContext->PSSetShader(m_pBackBufferPixelShader, 0, 0);
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->PSSetShaderResources(3, RENDERTARGET_COUNT, &m_pDeferredShaderResourceViews[0]);
	m_pContext->PSSetSamplers(0, 1, &m_pPointSampler);

	const unsigned stride = sizeof(FVEC4) + sizeof(FVEC2);
	const unsigned offset = 0;
	m_pContext->IASetVertexBuffers(0, 1, &m_pBackBufferVertexBuffer, &stride, &offset);
	m_pContext->IASetIndexBuffer(m_pBackBufferIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	// call to draw
	m_pContext->DrawIndexed(6, 0, 0);

	// reset
	ID3D11Buffer*				nullbuffer		= 0;
	ID3D11SamplerState*			nullsampler		= 0;
	ID3D11RenderTargetView*		nullview		= 0;
	ID3D11ShaderResourceView*	nullresources[] = { 0, 0, 0 };
	ID3D11VertexShader*			nullvs			= 0;
	ID3D11PixelShader*			nullps			= 0;

	m_pContext->OMSetDepthStencilState(nullptr, 0);
	m_pContext->RSSetState(nullptr);
	m_pContext->OMSetRenderTargets(1, &nullview, nullptr);
	m_pContext->IASetInputLayout(nullptr);
	m_pContext->VSSetShader(nullvs, 0, 0);
	m_pContext->PSSetShader(nullps, 0, 0);
	m_pContext->PSSetShaderResources(3, 3, nullresources);
	m_pContext->PSSetSamplers(0, 1, &nullsampler);
	m_pContext->IASetVertexBuffers(0, 0, nullptr, 0, 0);
	m_pContext->IASetIndexBuffer(nullbuffer, DXGI_FORMAT_R32_UINT, 0);
}