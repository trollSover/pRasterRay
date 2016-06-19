#include "RasterRayApp.h"

using namespace RasterRay;

/* note this is just a static message dispatcher to the instanced applications message handler */
LRESULT CALLBACK WndProcDispatch(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam)
{
	IApplication* pSelf = nullptr;
	switch (_msg)
	{
	case WM_NCCREATE:
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(_lparam);
		pSelf = reinterpret_cast<IApplication*>(lpcs->lpCreateParams);
		SetWindowLongPtr(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf));
	}
	break;
	default:
		pSelf = reinterpret_cast<IApplication*>(GetWindowLongPtr(_hwnd, GWLP_USERDATA));
		break;
	}
	if (pSelf) return pSelf->VApplicationProc(_hwnd, _msg, _wparam, _lparam);	// this is the instanced message handler
	else return DefWindowProc(_hwnd, _msg, _wparam, _lparam);
}

bool RasterRayApp::ReloadAppConfig(void)
{
	std::ifstream appStream("app.cfg");
	if (!appStream.is_open())
	{
		PrintError(AT, "app config file not found!");
		return false;
	}


	std::string str = "";
	const char delim = '=';
	std::string key, val;
	while (std::getline(appStream, str))
	{
		if (str.length() == 0)	// empty line
			continue;
		if (str.find(delim) == std::string::npos)	// no delim
			continue;
		if (str.find('#') != std::string::npos)	// line is comment
			continue;

		int sep = str.find(delim);
		key = str.substr(0, sep);
		val = str.substr(sep + 1, str.length());
		m_appCmd[key] = val;
	}

	// ugly quickfix for raycaster dispatch count
	try
	{
		m_dispatchX = std::stoi(m_appCmd["WORK_SIZE_X"]);
		m_dispatchY = std::stoi(m_appCmd["WORK_SIZE_Y"]);
	}
	catch (std::exception e)
	{
		PrintError(AT, "app config file - computeshader WORK_SIZE_* corrupt or missing!");
		return false;
	}

	return true;
}

bool RasterRayApp::VInit(void)
{
	if (!ReloadAppConfig())
	{
		return false;
	}

	try
	{
		m_resWidth = std::stoi(m_appCmd["width"]);
		m_resHeight = std::stoi(m_appCmd["height"]);
	}
	catch (std::exception e)
	{
		PrintError(AT, "app config file - no resolution defined!");
		return false;
	}

	m_hInstance = GetModuleHandle(0);

	HRESULT hr			= S_OK;
	bool res			= true;
	unsigned width		= VGetResolution().width;
	unsigned height		= VGetResolution().height;
	DXGI_FORMAT format	= DXGI_FORMAT_R8G8B8A8_UNORM;

	/* Register process */

	WNDCLASSEX wcex = { 0 };

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= WndProcDispatch;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInstance;
	wcex.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wcex.hIconSm		= wcex.hIcon;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(GRAY_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "RasterRay";
	wcex.cbSize			= sizeof(WNDCLASSEX);

	res = RegisterClassEx(&wcex);
	if (!res) { PrintError(AT, "failed to register process"); return false; }

	/* Register window */
	unsigned dw_style	= WS_OVERLAPPEDWINDOW;
	unsigned ex_style	= WS_EX_APPWINDOW;
	unsigned x			= (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	unsigned y			= (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	RECT rect			= { 0, 0, width, height };
	AdjustWindowRectEx(&rect, dw_style, 0, ex_style);
	x		+= rect.left;
	y		+= rect.top;
	width	= rect.right - rect.left;
	height	= rect.bottom - rect.top;

	m_hwnd = CreateWindowEx(ex_style, "RasterRay", "RasterRay",
							dw_style,
							x, y, width, height,
							0, 0, m_hInstance, this);

	if (!m_hwnd) { PrintError(AT, "failed to create window"); return false; }

	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);
	ShowCursor(true);

	hr = CoInitialize(0);
	if (FAILED(hr)) { PrintError(AT, hr); return false; }

	/* Initialize dxgi envirnoment */
	hr = InitializeDXGI(VGetResolution().width, VGetResolution().height, format);
	if (FAILED(hr)) return false;

	/* create d3d devices */
	hr = CreateDeviceAndContext();
	if (FAILED(hr)) return false;

	/* create swap chain */
	hr = CreateSwapChain(m_displayMode, m_hwnd);
	if (FAILED(hr)) return false;

	/* create backbuffer environment */
	hr = CreateBackBufferEnvironment(format);
	if (FAILED(hr)) return false;

	/* create deferred environment */
	hr = CreateDeferredEnvironment(format);
	if (FAILED(hr)) return false;

	hr = CreateRasterizer(format);
	if (FAILED(hr)) return false;

	/* create input */
	m_pInput = VNEW SimpleInput();
	if (!m_pInput) { PrintError(AT, "failed to allocate input"); return false; }

	res = m_pInput->VInit();
	if (!res) { PrintError(AT, "failed to initialize input"); return false; }

	/* create camera */
	m_pCamera = VNEW XMCamera();
	if (!m_pCamera) { PrintError(AT, "failed to allocate camera"); return false; }

	res = m_pCamera->Init(VGetResolution().width, VGetResolution().height);
	if (!res) { PrintError(AT, "failed to initialize camera"); return false; }

	/* create font wrapper */
	IFW1Factory* pfw1Factory = nullptr;
	hr = FW1CreateFactory(FW1_VERSION, &pfw1Factory);
	if (FAILED(hr)) { PrintError(AT, hr); return false; }

	hr = pfw1Factory->CreateFontWrapper(m_pDevice, L"Times New Roman", &m_pFontWrapper);
	if (FAILED(hr)) { PrintError(AT, hr); return false; }

	pfw1Factory->Release();

	/* load volume data */
	std::ifstream stream("master64.txt");
	if (!stream)
	{
		PrintError(AT, "failed to open master config!");
		return false;
	}
	std::string line, path;
	while (std::getline(stream, line))
	{
		if (line.find_first_of('#') == 0) continue;	// whole line commented out
		if (line.find("source") != std::string::npos)
		{
			path = line.erase(0, line.find_first_of(' ') + 1);
			break;
		}
	}

	if (!m_sparseOctree.LoadFile(path))
		return false;

	const unsigned lodNo = m_sparseOctree.GetHeader().numLevels;
	m_pRasterizerGeometryVertexBuffers.resize(lodNo);

	D3D11_BUFFER_DESC rvd = { 0 };
	rvd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	rvd.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA rvsrd = { 0 };

	for (unsigned i = 0; i < lodNo; ++i)
	{
		std::vector<Vertex> lodVertices = m_sparseOctree.GetLodVertices(i);
		rvd.ByteWidth = sizeof(Vertex) * lodVertices.size();
		rvsrd.pSysMem = &lodVertices[0];

		hr = m_pDevice->CreateBuffer(&rvd, &rvsrd, &m_pRasterizerGeometryVertexBuffers[i]);
		if (hr != S_OK) { PrintError(AT, hr); return hr; }
	}

	/***********************************
	--- Post creation initialization ---
	************************************/

	/* Create debug render environment */
	m_debugGridUnitSize = m_sparseOctree.GetHeader().numLevels;
	m_debugGridLength = m_sparseOctree.GetHeader().gridlength / m_debugGridUnitSize;
	hr = CreateDebugEnvironment(m_debugGridLength, m_debugGridUnitSize);
	if (FAILED(hr)) { PrintError(AT, hr); return false; }

	m_pDebugBoundingVolumeBuffer.resize(m_sparseOctree.m_boundingGrid.size());
	m_pDebugBoundingVolumeIndices.resize(m_sparseOctree.m_boundingGridIndices.size());
	for (unsigned i = 0; i < m_sparseOctree.m_boundingGrid.size(); ++i)
	{
		/* vertex buffer */
		D3D11_BUFFER_DESC gridDesc = { 0 };
		gridDesc.Usage = D3D11_USAGE_IMMUTABLE;
		gridDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		gridDesc.CPUAccessFlags = 0;
		gridDesc.StructureByteStride = sizeof(FVEC3);
		gridDesc.ByteWidth = sizeof(FVEC3) * m_sparseOctree.m_boundingGrid[i].size();

		D3D11_SUBRESOURCE_DATA gridSRD = { 0 };
		gridSRD.pSysMem = &m_sparseOctree.m_boundingGrid[i][0];

		hr = m_pDevice->CreateBuffer(&gridDesc, &gridSRD, &m_pDebugBoundingVolumeBuffer[i]);
		if (FAILED(hr)) { PrintError(AT, hr); return hr; }

		/* index buffer */
		gridDesc = { 0 };
		gridDesc.Usage = D3D11_USAGE_IMMUTABLE;
		gridDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		gridDesc.StructureByteStride = sizeof(unsigned);
		gridDesc.ByteWidth = sizeof(unsigned) * m_sparseOctree.m_boundingGridIndices[i].size();

		gridSRD = { 0 };
		gridSRD.pSysMem = &m_sparseOctree.m_boundingGridIndices[i][0];

		hr = m_pDevice->CreateBuffer(&gridDesc, &gridSRD, &m_pDebugBoundingVolumeIndices[i]);
		if (FAILED(hr)) { PrintError(AT, hr); return hr; }
	}

	/* Create Raycast render environment */
	hr = CreateRaycaster();
	if (FAILED(hr)) { PrintError(AT, hr); return false; }

	/* Camera */
	m_pCamera->SetPositionAndView(0, 0, 0, 0, 0);
	m_pCamera->Update(Time());

	return true;
}