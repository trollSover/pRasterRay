#include "RasterRayApp.h"

using namespace RasterRay;

HRESULT	RasterRayApp::InitializeDXGI(const unsigned _width, const unsigned _height, const DXGI_FORMAT _format)
{
	/* I'm leaving this very verbose so no API call params are unclear */

	HRESULT hr = S_OK;

	/* Set up factory */
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&m_pDxgiFactory);

	if (FAILED(hr))
	{
		PrintError(AT, hr);
		return hr;
	}

	/* Set up adapter */
	const unsigned adapterId = 0;	// graphics adapter (hardware/integrated gpu) index - in our case we just choose the default/main
	hr = m_pDxgiFactory->EnumAdapters(adapterId, &m_pDxgiAdapter);

	if (FAILED(hr))
	{
		PrintError(AT, hr);
		return hr;
	}

	/* Set up desired output */
	const unsigned monitorId = 0;	// display monitor index - in our case we just choose the default/main
	hr = m_pDxgiAdapter->EnumOutputs(monitorId, &m_pDxgiOutput);

	if (FAILED(hr))
	{
		PrintError(AT, hr);
		return hr;
	}

	/* grab the adapter description while we're at it */
	DXGI_ADAPTER_DESC adapterDesc;
	hr = m_pDxgiAdapter->GetDesc(&adapterDesc);

	if (FAILED(hr))
	{
		PrintError(AT, hr);
		return hr;
	}
	// get vram
	m_adapterMemory = adapterDesc.DedicatedVideoMemory / 1024 / 1024;

	// do cryptic wchar -> char array conversion to get the adapter name
	char ch[128];
	char defChar = ' ';
	WideCharToMultiByte(CP_ACP, 0, adapterDesc.Description, -1, ch, 128, &defChar, 0);

	m_adapterName = std::string(ch);

	/* Grab the supported display modes of the output */
	unsigned modeCount = 0;	// amount of supported display modes for the selected output
	hr = m_pDxgiOutput->GetDisplayModeList(_format, DXGI_ENUM_MODES_INTERLACED, &modeCount, 0);

	if (FAILED(hr))
	{
		PrintError(AT, hr);
		return hr;
	}

	/* Create and populate display mode list array */
	DXGI_MODE_DESC* pDisplayModes = new DXGI_MODE_DESC[modeCount];
	if (!pDisplayModes)
	{
		PrintError(AT, "failed to allocate heap memory");
		return S_FALSE;
	}

	hr = m_pDxgiOutput->GetDisplayModeList(_format, DXGI_ENUM_MODES_INTERLACED, &modeCount, pDisplayModes);

	/* iterate display modes to find matching output
		- in our case, instead of hardcoding the refresh rate,
		we iterate until we find the matching output with the best
		refresh rate 
		note: these are unsigned but we treat them as doubles here for easy
		division comparisons since some combinations are fractions */

	/* these are our refresh rate variables */
	double numerator = 1;
	double denominator = 1;

	for (unsigned i = 0; i < modeCount; ++i)
	{
		if (pDisplayModes[i].Width == _width && pDisplayModes[i].Height == _height)
		{
			// resolution checks out
			double tNumerator = pDisplayModes[i].RefreshRate.Numerator;
			double tDenominator = pDisplayModes[i].RefreshRate.Denominator;

			if ((numerator / denominator) < (tNumerator / tDenominator))
			{
				// new refresh rate is higher than current rate
				numerator	= tNumerator;
				denominator = tDenominator;

				m_displayMode = pDisplayModes[i];
			}
		}
	}

	/* clean up allocated memory before moving on */
	delete[] pDisplayModes;

	/* this is not a pretty sanity check but we just want to ensure that nothing
	funky is going on with the setup */
	if ((numerator / denominator) < 30)
	{
		PrintError(AT, "output refresh rate < 30 hz");
		return S_FALSE;
	}

	return S_OK;
}

HRESULT	RasterRayApp::CreateSwapChain(const DXGI_MODE_DESC& _displayMode, HWND _hwnd)
{
	HRESULT hr = S_OK;

	DXGI_SWAP_CHAIN_DESC scd = { 0 };
	scd.BufferCount = 1;
	scd.BufferDesc.Width					= _displayMode.Width;
	scd.BufferDesc.Height					= _displayMode.Height;
	scd.BufferDesc.Format					= _displayMode.Format;
	scd.BufferDesc.RefreshRate.Numerator	= _displayMode.RefreshRate.Numerator;
	scd.BufferDesc.RefreshRate.Denominator	= _displayMode.RefreshRate.Denominator;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = _hwnd;
	scd.SampleDesc.Count = 1;

	scd.Windowed = true;	// always create with true if multiple swap chains

	/* note: for D3D_FEATURE_LEVEL_11_1 use CreateSwapChainForHwnd instead (or be lazy and use the d3d11::CreateswapchainANDdevice(...) */
	IDXGIDevice* pDevice = nullptr;
	m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDevice);
	hr = m_pDxgiFactory->CreateSwapChain(pDevice, &scd, &m_pSwapChain);

	if (FAILED(hr))
	{
		PrintError(AT, hr);
	}

	return hr;
}