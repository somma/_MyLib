/**
 * @file    _test_steady_timer.h
 * @brief   Test for SteadyTimer class
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2018/09/28 13:58 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#include "stdafx.h"
#include <crtdbg.h>
#include "_MyLib/src/steady_timer.h"

bool test_steady_timer_using_lambda()
{
	//
	//	1초 짜리 타이머를 생성
	//
	DWORD_PTR random_value = 1024;
	SteadyTimer timer;
	bool ret = timer.start(2, 
						   random_value, 
						   [&](_In_ DWORD_PTR tag)->bool
	{
		_ASSERTE(random_value == tag);
		log_info "timer callback" log_end;
		return true;
	});
	_ASSERTE(true == ret);
	
	for (int i = 0; i < 4; ++i)
	{
		Sleep(1000);		
	}

	timer.stop();
	return true;
}


bool timer_callback(_In_ DWORD_PTR tag)
{
	UNREFERENCED_PARAMETER(tag);

	//
	//	4초 이후에 타이머를 취소한다. 
	//
	static uint32_t count = 0;

	count++;
	log_info "timer callback: %u", count log_end;

	if (count == 4)
		return false;
	else
		return true;
}

bool test_steady_timer_using_callback()
{
	//
	//	1초 짜리 타이머를 생성
	//	
	DWORD_PTR random_value = 1024;
	SteadyTimer timer;
	bool ret = timer.start(1,
						   random_value,
						   timer_callback);
	_ASSERTE(true == ret);

	for (int i = 0; i < 3; ++i)
	{
		Sleep(1000);
	}
	timer.stop();
	return true;
}

class SteadyTimerTest
{
public:
	SteadyTimerTest()
	{
	}

	bool callback(_In_ DWORD_PTR tag)
	{
		UNREFERENCED_PARAMETER(tag);		
		_count++;
		log_info "timer callback: %u", _count log_end;
		return true;
	}

	bool start()
	{
		//
		//	1초 짜리 타이머를 생성
		//	
		_count = 0;
		bool ret = _timer.start(1, _count, boost::bind(&SteadyTimerTest::callback,
													   this, 
													   boost::placeholders::_1));
		_ASSERTE(true == ret);
		return true;
	}

	void stop()
	{
		_timer.stop();
	}

private:
	SteadyTimer _timer;
	uint32_t _count;
};

bool test_steady_timer_class()
{
	SteadyTimerTest timer;
	_ASSERTE(true == timer.start());
	Sleep(5000);
	timer.stop();
	return true;
}

bool on_timer(_In_ DWORD_PTR tag)
{
	UNREFERENCED_PARAMETER(tag);
	int timer_number = (int)tag;

	log_info "timer[%d]: callback", timer_number log_end;

	return true;
}

bool test_steady_timer_restart()
{
	SteadyTimer timer;

	for (int i = 0; i < 3; ++i)
	{
		log_info "timer[%d]: start...", i log_end;
		if (!timer.start(1, i, on_timer))
		{
			log_err "timer.start() failed." log_end;
			return false;
		}
		
		Sleep(6000);
		log_info "timer[%d]: stop...", i log_end;
		timer.stop();
		Sleep(2000);
	}
	_pause;
	return true;
}

bool test_steady_timer()
{
	//_ASSERTE(true == test_steady_timer_using_lambda());
	//_ASSERTE(true == test_steady_timer_using_callback());
	//_ASSERTE(true == test_steady_timer_class());
	_ASSERTE(true == test_steady_timer_restart());
	return true;
}