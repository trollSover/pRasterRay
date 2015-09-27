#pragma once

#include "CoreStd.h"
#include "Window.h"
#include "ITimer.h"

class IApplication;

class Core
{
private:
	IApplication*	m_pApplication;
	ITimer*			m_pTimer;

	Window		m_window;
	Resolution	m_resolution;

protected:
public:

private:
protected:
public:
	Core();
	~Core();

	bool Init(IApplication* _application, HINSTANCE _hInstance);
	void Close();
	void Run();

	HWND	GetHwnd() { return m_window.GetHwnd(); }
};