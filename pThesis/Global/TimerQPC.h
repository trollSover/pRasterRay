#pragma once

#include "../CoreSystems/ITimer.h"

class TimerQPC
	: public ITimer
{
private:
	Time m_time;
	LARGE_INTEGER origin, start, frequency;

protected:
public:

private:
	TimerQPC(const TimerQPC&) = delete;
protected:
	
public:
	TimerQPC() {}
	~TimerQPC() {}

	bool VInit()	final;
	Time VGetTime() final;
	void VUpdate()	final;
};