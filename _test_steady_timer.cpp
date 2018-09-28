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
#include "steady_timer.h"

bool test_steady_timer_using_lambda()
{
	//
	//	1초 짜리 타이머를 생성
	//
	DWORD_PTR random_value = 1024;
	SteadyTimer timer;
	bool ret = timer.start(1, 
						   random_value, 
						   [](_In_ DWORD_PTR tag)->bool
	{
		_ASSERTE(1024 == tag);
		log_info "timer callback" log_end;
		return true;
	});
	_ASSERTE(true == ret);
	
	for (int i = 0; i < 3; ++i)
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

bool test_steady_timer()
{
	_ASSERTE(true == test_steady_timer_using_lambda());
	_ASSERTE(true == test_steady_timer_using_callback());
	return true;
}