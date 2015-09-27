#include "TimerQPC.h"

bool TimerQPC::VInit()
{
	if (!QueryPerformanceFrequency(&frequency))
		return false;
	QueryPerformanceCounter(&origin);
	start = origin;
	return true;
}

Time TimerQPC::VGetTime()
{
	LARGE_INTEGER stop;
	QueryPerformanceCounter(&stop);

	double dtMs	= (double)(stop.QuadPart - start.QuadPart) / frequency.QuadPart * 1000.0;
	double totS = (double)(stop.QuadPart - origin.QuadPart) / frequency.QuadPart;

	return Time(dtMs, totS);
}

void TimerQPC::VUpdate()
{
	QueryPerformanceCounter(&start);
}