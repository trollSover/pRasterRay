#pragma once

#include "CoreStd.h"

class IApplication
{
private:
	IApplication(const IApplication&) = delete;
protected:
	IApplication() {}

public:
	virtual ~IApplication() {}

	virtual bool VInit()			= 0;
	virtual HRESULT VFrame(Time time)	= 0;

	virtual			Resolution	VGetResolution()const = 0;
	virtual const	LPCSTR		VGetAppName()	const = 0;

	virtual LRESULT VApplicationProc(HWND _hwnd, UINT _umsg, WPARAM _wparam, LPARAM _lparam) = 0;
};