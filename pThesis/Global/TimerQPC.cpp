#include "TimerQPC.h"

bool TimerQPC::VInit()
{
	//if (!QueryPerformanceFrequency(&frequency))
	//	return false;
	//QueryPerformanceCounter(&origin);
	//start = origin;
	//return true;

	QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
	if (m_frequency == 0)
	{
		return false;
	}

	// Find out how many times the frequency counter ticks every millisecond.
	m_ticksPerMS = (float)(m_frequency / 1000.f);

	QueryPerformanceCounter((LARGE_INTEGER*)&m_start);
	return true;
}

Time TimerQPC::VGetTime()
{
	//LARGE_INTEGER stop;
	//QueryPerformanceCounter(&stop);

	//double dtMs	= (double)(stop.QuadPart - start.QuadPart) /  (double)frequency.QuadPart * 1000.0;
	//double totS = (double)(stop.QuadPart - origin.QuadPart) / (double)frequency.QuadPart;

	//return Time(dtMs, totS);
	return Time(m_frameTime, 0);
}

void TimerQPC::VUpdate()
{
	//QueryPerformanceCounter(&start);

	INT64 currentTime;
	float timeDifference;


	QueryPerformanceCounter((LARGE_INTEGER*)& currentTime);

	timeDifference = (float)(currentTime - m_start);

	m_frameTime = timeDifference / m_ticksPerMS;

	m_start = currentTime;
}