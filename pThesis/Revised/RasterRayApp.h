#pragma once

#include "../CoreSystems/IApplication.h"
#include "../Global/SimpleInput.h"
#include "../include/FW1FontWrapper.h"
#include "../Global/XMCamera.h"
#include "SparseOctree.h"
#include "ConstantBufferDefines.h"
#include "Frustum.h"
#include <atlbase.h>
#include <dxgi.h>
#include <wincodec.h>

namespace RasterRay
{


	class RasterRayApp
		: public IApplication
	{
		/* Members */
	private:
		enum Deferred_RenderTarget_Id
		{
			DRTI_Color	= 0,
			DRTI_Normal = 1,
			DRTI_Depth	= 2
		};

		enum RasterState_Id
		{
			RSI_CNFS = 0,
			RSI_CNFW,
			RSI_CBFS,
			RSI_CBFW,
			RSI_CFFS,
			RSI_CFFW,
			RSI_COUNT
		};

		enum DepthStencilState_Id
		{
			DSSI_ENABLED = 0,
			DSSI_DISABLED,
			DSSI_COUNT
		};

		struct RasterState
		{
			ID3D11RasterizerState* m_pState;
			std::string cull, fill;
		};

		static const unsigned RENDERTARGET_COUNT	= 3;
		static const unsigned RASTERSTATE_COUNT		= 6;
		/* Application specific */
		HINSTANCE				m_hInstance;
		HWND					m_hwnd;
		ID3D11Device			*m_pDevice;
		ID3D11DeviceContext		*m_pContext;
		IDXGIFactory			*m_pDxgiFactory;
		IDXGIAdapter			*m_pDxgiAdapter;
		IDXGIOutput				*m_pDxgiOutput;
		IDXGISwapChain			*m_pSwapChain;

		/* Global render settings */
		ID3D11RasterizerState	*m_pRasterState;
		ID3D11Texture2D			*m_pDepthStencilTexture;
		ID3D11DepthStencilView	*m_pDepthStencilView;
		ID3D11DepthStencilState*	m_pDepthStencilStates[DSSI_COUNT];
		//ID3D11DepthStencilState	*m_pDepthStencilEnabled;
		//ID3D11DepthStencilState	*m_pDepthStencilDisabled;
		ID3D11BlendState		*m_pBlendState;
		RasterState	m_pRasterizerStates[RSI_COUNT];
		unsigned m_currentDepthStateIndex;
		unsigned m_currentRasterStateIndex;


		ID3D11SamplerState		*m_pPointSampler;
		ID3D11Buffer			*m_pGlobalCameraConstantBuffer;
		ID3D11Buffer			*m_pGlobalWindowConstantBuffer;

		/* Backbuffer render data */
		ID3D11Texture2D			*m_pBackBufferTexture;
		ID3D11RenderTargetView	*m_pBackBufferRenderTargetView;	
		ID3D11VertexShader		*m_pBackBufferVertexShader;
		ID3D11PixelShader		*m_pBackBufferPixelShader;
		ID3D11InputLayout		*m_pBackBufferInputLayout;
		ID3D11Buffer			*m_pBackBufferVertexBuffer;
		ID3D11Buffer			*m_pBackBufferIndexBuffer;

		/* Deferred render data */
		ID3D11RenderTargetView		*m_pDeferredRenderTargetViews[RENDERTARGET_COUNT];
		ID3D11ShaderResourceView	*m_pDeferredShaderResourceViews[RENDERTARGET_COUNT];
		ID3D11UnorderedAccessView	*m_pDeferredUnorderedAccessViews[RENDERTARGET_COUNT];
		ID3D11Texture2D				*m_pDeferredTextures[RENDERTARGET_COUNT];

		/* Raycast render data */
		ID3D11Buffer				*m_pRaycastOctreeConstantBuffer;
		ID3D11Buffer				*m_pRaycastNodeBuffer;
		ID3D11Buffer				*m_pRaycastVoxelBuffer;
		ID3D11UnorderedAccessView	*m_pRaycastNodeUAV;
		ID3D11UnorderedAccessView	*m_pRaycastVoxelUAV;
		ID3D11ComputeShader			*m_pRayCastComputeShader;

		/* Rasterizer render data */
		ID3D11VertexShader			*m_pRasterizerVertexShader;
		ID3D11PixelShader			*m_pRasterizerPixelShader;
		ID3D11InputLayout			*m_pRasterizerGeometryInputLayout;
		std::vector<ID3D11Buffer*>	m_pRasterizerGeometryVertexBuffers;
		unsigned					m_lodLevel;

		
		/* Debug render data */
		unsigned m_debugGridLength, m_debugGridUnitSize;
		ID3D11Buffer		*m_pDebugGridConstantBuffer;
		ID3D11Buffer		*m_pDebugGridVertexBuffer;
		ID3D11Buffer		*m_pDebugGridIndexBuffer;
		ID3D11InputLayout	*m_pDebugGridInputLayout;
		ID3D11VertexShader	*m_pDebugGridVertexShader;
		ID3D11PixelShader	*m_pDebugGridPixelShader;
		std::vector<ID3D11Buffer*> m_pDebugBoundingVolumeBuffer;
		std::vector<ID3D11Buffer*> m_pDebugBoundingVolumeIndices;
		
		
		DXGI_MODE_DESC					m_displayMode;
		std::string						m_adapterName;
		unsigned						m_adapterMemory;

		SimpleInput*					m_pInput;
		IFW1FontWrapper*				m_pFontWrapper;
		XMCamera*						m_pCamera;

		D3D11_VIEWPORT					m_viewPort;
		D3D11_FILL_MODE					m_fillMode;
		D3D11_CULL_MODE					m_cullMode;

		
		bool							m_drawDebug;
		bool							m_drawRasterized;
		bool							m_drawRayCast;
		bool							m_drawUIHelp;
		bool							m_drawUI;
		unsigned						m_FPS;

		/* Bounding Volume */
		SparseOctree m_sparseOctree;

		Frustum	m_frustum;
		OctreeConstantBuffer m_raycastSceneCB;

		/* User Config */
		std::map<std::string, std::string> m_appCmd;
		unsigned m_resWidth, m_resHeight;
		unsigned m_dispatchX, m_dispatchY;
		
		/* Methods */
	private:
		unsigned GetRasterStateId(const D3D11_FILL_MODE, const D3D11_CULL_MODE);
		void DrawDebug(void);
		void DrawRasterized(void);
		void DrawRayCast(void);
		void DrawUI(const Time&);
		void DrawToBackBuffer(void);
		void UpdateCamera(const Time&);
		void UpdateUI(const Time&);
		void UpdateGlobalShaderBuffers(void);

		HRESULT ReloadShader(const std::string);

		HRESULT CompileShader(LPCWSTR, LPCSTR, ID3D10Blob*&, LPCSTR, D3D_SHADER_MACRO* = nullptr);
		HRESULT	InitializeDXGI(const unsigned, const unsigned, const DXGI_FORMAT);
		HRESULT CreateSwapChain(const DXGI_MODE_DESC&, HWND);
		HRESULT CreateDeviceAndContext(void);
		HRESULT CreateBackBufferEnvironment(const DXGI_FORMAT);
		HRESULT CreateDeferredEnvironment(const DXGI_FORMAT);
		HRESULT CreateRasterStates(void);
		HRESULT CreateRasterizer(const DXGI_FORMAT);
		HRESULT CreateDebugEnvironment(const signed,const unsigned);
		HRESULT CreateRaycaster(void);

		void ReleaseDXGI(void);
		void ReleaseSwapChain(void);
		void ReleaseDeviceAndContext(void);
		void ReleaseBackBufferEnvironment(void);
		void ReleaseDeferredEnvironment(void);
		void ReleaseRasterStates(void);
		void ReleaseRasterizer(void);
		void ReleaseDebugEnvironment(void);
		void ReleaseRayCaster(void);
		
		unsigned SetFillMode(const D3D11_FILL_MODE&);
		unsigned SetCullMode(const D3D11_CULL_MODE&);
		unsigned SetDepthMode(const unsigned);
		void ResetRasterState(void);
		void ResetDepthMode(void);
		bool IsKeyPress(unsigned);
		bool ToggleOnKey(unsigned, bool&);
		HRESULT TakeScreenShot(ID3D11Texture2D*);

		void UpdateRenderState(void);

		bool ReloadAppConfig(void);

	public:
		RasterRayApp(void);
		~RasterRayApp(void);

		virtual bool			VInit(void)	override;
		virtual	HRESULT			VFrame(Time)override;
		virtual const	LPCSTR	VGetAppName(void) const	override { return "RasterRay"; }
		virtual	Resolution		VGetResolution()const override { return { m_resWidth, m_resHeight}; }
		virtual	LRESULT			VApplicationProc(HWND, unsigned, WPARAM, LPARAM) override;

		static void RunApplication(IApplication*);
	};

	
}