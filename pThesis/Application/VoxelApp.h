#pragma once

#include "../CoreSystems/IApplication.h"
#include "../Global/SimpleInput.h"
#include "../CoreSystems/DXDriver.h"
#include "../Render/Rasterizer.h"
#include "../Render/RayCaster.h"
#include "../Render/BackBufferRender.h"
#include "../Render/Font.h"
#include "../Render/Overlay.h"
#include "../Render/Buffer.h"
#include "../Render/Culling.h"
#include "../Global/XMCamera.h"
#include "../Global/DXCamera.h"

#include "SVO.h"

#include "../Render/RenderTarget.h"

#include "../include/FW1FontWrapper.h"

#include "PerfTracker.h"
#include "../RaycastEmulate.h"
//static const int RENDERTARGET_COUNT = 3;

enum RENDER_TYPE
{
	RT_Rasterizer	= 0,
	RT_Raytracer	= 1,
	RT_Debug		= 2,
	RT_Unknown		= 3,
};

class VoxelApp
	: public IApplication
{
private:
	CPURaycaster rcEmulate;


	std::vector<uint32_t>	rasterIndices;
	AmdPerf::PerfContext	m_renderloopPerfContext;

	RENDER_TYPE	m_eRenderType;

	SimpleInput*	m_pInput;
	DXDriver*		m_pDriver;

	IRenderer*	m_pRasterizer;
	IRenderer*	m_pRayCaster;
	IRenderer*	m_pDebugRenderer;
	IRenderer*	m_pBackBufferDeferred;

	XMCamera	m_camera;
	DXCamera*	m_dxCamera;

	/* Constant Buffers */
	D3DBuffer*	m_pCameraCB;
	D3DBuffer*	m_pVoxelCB;
	D3DBuffer*	m_pWindowCB;
	D3DBuffer*	m_pEvalCB;
	D3DBuffer*	m_pOctreeMatrices;

	/* Resource Buffers */
	D3DBuffer*	m_pTerrainVBuffer;
	D3DBuffer*	m_pTerrainIBuffer;
	D3DBuffer*	m_pVoxelBuffer;
	D3DBuffer*	m_pNodeBuffer;

	D3DBuffer*	m_pDebugVBuffer;

	FrustumCulling*	m_pCuller;

	RenderTarget* m_pRenderTargets[RENDERTARGET_COUNT];


	SVO_Loader	m_svoLoader;
	SparseVoxelOctree<NC>	m_svo;

	CBDCamera	cbCamera;
	CBDVoxel	cbVoxel;
	CBDWindow	cbWindow;
	CBDEval		cbEval;
	CBOctreeMatrices	cbOctreeMatrices;

	D3D11_FILL_MODE m_fillMode;
	D3D11_CULL_MODE	m_cullMode;

	IFW1FontWrapper* m_pFontWrapper;

	bool drawRasterized;
	bool drawRayCast;
	bool drawUIHelp;
	bool drawUI;

protected:
public:

private:
	~VoxelApp();

	void SetRenderType(RENDER_TYPE _renderType);

	void SetFillMode(const D3D11_FILL_MODE&);
	void SetCullMode(const D3D11_CULL_MODE&);

	bool KeyPress(unsigned int);
	bool ToggleOnKey(unsigned int, bool&);
	
	void UpdateCamera(const Time&);
	void UpdateRenderState(void);
	void UpdateBuffers(void);
	void UpdateUI(const Time&);
	void RenderGeometry(void);
	void RenderUI(const Time&);
protected:
public:
	bool VInit()			final;
	HRESULT VFrame(Time time)	final;

	Resolution		VGetResolution()	const final;
	const LPCSTR	VGetAppName()		const final;

	LRESULT VApplicationProc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam) final;
};