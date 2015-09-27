#include "Window.h"
#include "IApplication.h"

extern IApplication* g_pApplication;

LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam);


bool Window::Init(HINSTANCE _hInstance)
{
	m_hInstance = _hInstance;

	WNDCLASSEX wcex;
	
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= WindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInstance;
	wcex.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wcex.hIconSm		= wcex.hIcon;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= g_pApplication->VGetAppName();
	wcex.cbSize			= sizeof(WNDCLASSEX);

	RegisterClassEx(&wcex);

	Resolution res = g_pApplication->VGetResolution();
	uint x, y;
	//if g_pApp.fullscreen do:
	/*...
	else*/
	{
		x = (GetSystemMetrics(SM_CXSCREEN) - res.width) / 2;
		y = (GetSystemMetrics(SM_CYSCREEN) - res.height) / 2;
	}

	m_hWnd = CreateWindowEx(WS_EX_APPWINDOW, g_pApplication->VGetAppName(), g_pApplication->VGetAppName(),
							WS_CLIPSIBLINGS | WS_CLIPCHILDREN , 
							x, y, res.width, res.height, 
							NULL, NULL, m_hInstance, NULL);

	if (!m_hWnd)
	{
		return false;
	}

	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	ShowCursor(true);
	
	return true;
}

void Window::Close()
{
	DestroyWindow(m_hWnd);
	m_hWnd = NULL;

	UnregisterClass(g_pApplication->VGetAppName(), m_hInstance);
	m_hInstance = NULL;
}

LRESULT CALLBACK WindowProc(HWND _hwnd, UINT _msg, WPARAM _wparam, LPARAM _lparam)
{
	switch (_msg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		default:
			return g_pApplication->VApplicationProc(_hwnd, _msg, _wparam, _lparam);
	}
	return 0;
}