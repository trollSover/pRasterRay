#pragma once

#include "CoreStd.h"

class ITimer
{
private:
	ITimer(const ITimer&) = delete;
protected:
	ITimer() {}
public:
	virtual ~ITimer() {}

	virtual bool VInit()	= 0;
	virtual Time VGetTime() = 0;
	virtual void VUpdate()	= 0;

};