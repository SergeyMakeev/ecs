#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


namespace Time
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline __int64 GetTime()
	{
		LARGE_INTEGER largeInteger;
		QueryPerformanceCounter( &largeInteger );
		return largeInteger.QuadPart;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline __int64 GetFrequency()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return frequency.QuadPart;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline __int64 GetTimeMilliSeconds()
	{
		LARGE_INTEGER largeInteger;
		QueryPerformanceCounter( &largeInteger );
		return ( largeInteger.QuadPart * __int64(1000) ) / GetFrequency();
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline __int64 GetTimeMicroSeconds()
	{
		LARGE_INTEGER largeInteger;
		QueryPerformanceCounter( &largeInteger );
		return ( largeInteger.QuadPart * __int64(1000000) ) / GetFrequency();
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline void SpinSleep(size_t microSeconds)
	{
		__int64 time = GetTimeMicroSeconds() + microSeconds;
		while(GetTimeMicroSeconds() < time)
		{
			Sleep(0);
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
