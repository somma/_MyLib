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
#include <atomic>

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

class TimeAccumulator
{
public:
	TimeAccumulator(std::atomic<uint64_t>& elapsed): _elapsed(elapsed)
	{	
		_sw.Start();
	}

	~TimeAccumulator()
	{
		_sw.Stop();
		_elapsed.fetch_add(static_cast<uint64_t>(_sw.GetDurationMilliSecond()));
	}

private:
	StopWatch _sw;
	std::atomic<uint64_t>& _elapsed;
};