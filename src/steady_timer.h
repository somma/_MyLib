/**
 * @file    steady_timer.h
 * @brief   This class implements steady timer object.
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2018/09/28 13:58 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <ppl.h>
#include <ppltasks.h>

#include "log.h"


//	SteadyTimer callback, false 를 리턴하면 타이머를 중지
typedef boost::function<bool(DWORD_PTR tag)> SteadyTimerCallback;

typedef class SteadyTimer
{
public:
	SteadyTimer():	
		_running(false),
		_interval(0), 
		_tag(0),
		_callback(0), 
		_timer(_io_service)
	{
	}

	~SteadyTimer()
	{
		stop();
	}

	bool running() { return _running; }

	bool start(_In_ uint32_t interval_by_sec,
			   _In_ DWORD_PTR tag,
			   _In_ SteadyTimerCallback callback)
	{
		if (true == _running)
		{
			_ASSERTE(!"called twice");
			return false;
		}

		_interval = interval_by_sec;
		_tag = tag;
		_callback = callback;

		//
		//	타이머 객체 생성 
		//
		_timer.expires_from_now(std::chrono::seconds(this->_interval));
		_timer.async_wait(boost::bind(&SteadyTimer::internal_callback,
									 this,
									 boost::asio::placeholders::error,
									 boost::ref(_timer)));

		//
		//	타이머 시작, io_service_run() 은 blocking call 이므로 
		//	백그라운드 태스크를 만들어서 실행한다. 
		// 
		_timer_task = Concurrency::create_task([&]()->void {
			_io_service.reset();
			_io_service.run();
		});

		_running = true;
		return true;
	}

	void stop()
	{
		if (true != _running){return;}

		//
		//	타이머 취소
		//	
		_timer.cancel();
		_timer_task.wait();

		_interval = 0;
		_tag = 0;
		_callback = 0;
		_running = false;
	}

private:
	void internal_callback(_In_ const boost::system::error_code& error,
						   _In_ boost::asio::steady_timer& timer)
	{
		if (0 == error.value())
		{
			//
			//	callback 호출
			//
			_ASSERTE(true == _running);
			_ASSERTE(true != _callback.empty());
			if (true != _callback(_tag))
			{
				//
				//	타이머 취소 요청
				//
				timer.cancel();
			}
			else
			{
				//
				//	타이머 재 시작
				//				
				timer.expires_from_now(std::chrono::seconds(this->_interval));
				timer.async_wait(boost::bind(&SteadyTimer::internal_callback,
											 this,
											 boost::asio::placeholders::error,
											 boost::ref(timer)));
			}
		}
		else if (boost::asio::error::operation_aborted == error.value())
		{
			//
			//	timer 취소
			//	
			log_dbg "timer stop requested." log_end;
		}
		else
		{
			//
			//	에러가 나도 타이머는 재 시작한다. 
			log_err "timer error, err=0x%08x", error.value() log_end;
			timer.expires_from_now(std::chrono::seconds(1));
			timer.async_wait(boost::bind(&SteadyTimer::internal_callback,
										 this,
										 boost::asio::placeholders::error,
										 boost::ref(timer)));

		}
	}
private:
	bool _running;
	uint32_t _interval;	
	DWORD_PTR _tag;
	SteadyTimerCallback _callback;

	boost::asio::io_service _io_service;
	boost::asio::steady_timer _timer;
	concurrency::task<void> _timer_task;
} *PSteadyTimer;



