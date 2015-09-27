#pragma once

#include "CoreStd.h"

class IInput
{
private:
	IInput(const IInput&) = delete;
protected:
	IInput() {}
public:
	virtual ~IInput() {}

	virtual bool VInit()			= 0;
	virtual bool VIsKeyDown(uint)	= 0;
	virtual void VKeyDown(uint)		= 0;
	virtual void VKeyUp(uint)		= 0;
};