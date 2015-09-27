#pragma once

#include "CoreStd.h"

class Window
{
private:
	HINSTANCE	m_hInstance;
	HWND		m_hWnd;

protected:
public:

private:
protected:
public:
	bool Init(HINSTANCE _hInstance);
	void Close();

	HINSTANCE&	GetHinstance() { return m_hInstance; }
	HWND&		GetHwnd() { return m_hWnd; }
};