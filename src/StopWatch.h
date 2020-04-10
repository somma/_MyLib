/**----------------------------------------------------------------------------
 * StopWatch.h
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by somma (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 26:12:2011   17:53 created
 * - OpenMP 병렬 프로그래밍 by 정영훈 (CStopWatch class)
**---------------------------------------------------------------------------*/
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class StopWatch
{
private:
	LARGE_INTEGER	mFreq, mStart, mEnd;
	float			mTimeforDuration;
public :
	StopWatch()	: mTimeforDuration(0)
	{
		mFreq.LowPart = mFreq.HighPart = 0;
		mStart = mFreq;
		mEnd = mFreq;
		QueryPerformanceFrequency(&mFreq);
	}
	~StopWatch()
	{
	}

public:
	void Start(){ QueryPerformanceCounter(&mStart); }
	void Stop() 
	{ 
		QueryPerformanceCounter(&mEnd);	
		mTimeforDuration=(mEnd.QuadPart - mStart.QuadPart)/(float)mFreq.QuadPart;
	}
	float GetDurationSecond() { return mTimeforDuration; }
	float GetDurationMilliSecond() { return mTimeforDuration * 1000.f; }

};