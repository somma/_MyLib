/**----------------------------------------------------------------------------
 * _test_boost_asio_timer.cpp
 *-----------------------------------------------------------------------------
 * 
 *-----------------------------------------------------------------------------
 * All rights reserved by Noh,Yonghwan (fixbrain@gmail.com, unsorted@msn.com)
 *-----------------------------------------------------------------------------
 * 2014:12:27 17:44 created
**---------------------------------------------------------------------------*/
#include "stdafx.h"
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include <ppl.h>
#include <ppltasks.h>

void asio_on_timer(const boost::system::error_code& error);
bool test_boost_asio_timer_1();
bool test_boost_asio_timer_2();
bool test_boost_asio_timer_3();
bool test_boost_asio_timer_4();


/**
 * @brief	
 * @param	
 * @see		
 * @remarks	
 * @code		
 * @endcode	
 * @return	
**/
bool test_boost_asio_timer()
{
	test_boost_asio_timer_1();
	test_boost_asio_timer_2();
	test_boost_asio_timer_3();
	test_boost_asio_timer_4();
	return true;
}




/**
 * @brief	
**/
void asio_on_timer( const boost::system::error_code& error )
{
	if (!error)		// true if no error
	{
		log_info "timer called." log_end
	}
	else
	{
		log_err "error code = %d, msg = %s", error.value(), error.message().c_str() log_end
	}
}

/**
 * @brief	version 1 - 콜백 함수 이용
**/
bool test_boost_asio_timer_1()
{
	boost::asio::io_service io_service;

	boost::asio::steady_timer timer(io_service);	
	timer.expires_from_now(std::chrono::steady_clock::duration(1));
	
	timer.async_wait( asio_on_timer );
	io_service.run();
	return true;
}


/**
 * @brief	v2 - lambda 를 이용한 타이머
**/
bool test_boost_asio_timer_2()
{
	boost::asio::io_service io_service;

	boost::asio::steady_timer timer(io_service);
	timer.expires_from_now(std::chrono::steady_clock::duration(1));
	timer.async_wait( 
				[](const boost::system::error_code& error)
				{	
					if (!error)		// true if no error
					{
						log_info "timer called. "  log_end
					}
					else
					{
						log_err "error code = %d, msg = %s", error.value(), error.message().c_str() log_end
					}
				});
	io_service.run();

	return true;
}


/**
 * @brief	v3 - lamda + 타이머이벤트 계속 발생하기, 취소
**/
void print(const boost::system::error_code& error, boost::asio::steady_timer& timer, int& count)
{
	if (!error)
	{
		if (count < 5)
		{
			log_info "timer called, %d th call", count log_end;
			++(count);
			
			timer.expires_from_now(std::chrono::steady_clock::duration(1));
			timer.async_wait(boost::bind(print, boost::asio::placeholders::error, 
										 boost::ref(timer), 
										 boost::ref(count)));		
		}
		else
		{
			// cancel timer
			log_info "time canceled, %d th call", count log_end;
			timer.cancel();
		}
	}
	else if (boost::asio::error::operation_aborted == error.value())
	{
		std::cout << "timer canceled." << std::endl;
	}
	
}

bool test_boost_asio_timer_3()
{
	boost::asio::io_service io_service;

	int count = 0;
	boost::asio::steady_timer timer(io_service);
	timer.async_wait(boost::bind(print,
								 boost::asio::placeholders::error,
								 boost::ref(timer),
								 boost::ref(count)));
	io_service.run();
	log_info "done. final count=%d. ", count log_end
	return true;
}




/**
 * @brief	v3 - lamda + 타이머이벤트 계속 발생, 취소 ==> 클래스 버전
**/

class TimerClass
{
public:
	TimerClass() : count(0)
	{}
	~TimerClass()
	{}	

public: 
	volatile int count;

	void print(const boost::system::error_code& error, boost::asio::steady_timer& timer)
	{
		if (!error)
		{
			if (count < 5)
			{
				log_info "timer called, %d th call", count log_end;
				++(count);
			
				timer.expires_from_now(std::chrono::steady_clock::duration(1));
				timer.async_wait(boost::bind(&TimerClass::print, 
											 this,
											 boost::asio::placeholders::error,
											 boost::ref(timer)));		
			}
			else
			{
				// cancel timer
				log_info "time canceled, %d th call", count log_end;
				timer.cancel();
			}
		}
		else if (boost::asio::error::operation_aborted == error.value())
		{
			std::cout << "timer canceled." << std::endl;
		}	
	}

	bool start_timer()
	{
		boost::asio::io_service io_service;
		boost::asio::steady_timer timer(io_service);
		timer.async_wait(boost::bind(&TimerClass::print,
									 this, 
									 boost::asio::placeholders::error,
									 boost::ref(timer)));
		io_service.run();
		log_info "done. final count=%d. ", count log_end;
		return true;
	}


public : 
	boost::asio::steady_timer* _forever_timer;
	
	void print_forever(const boost::system::error_code& error, boost::asio::steady_timer* timer)
	{
		if (!error)
		{
			log_info "timer called, %d th call", count log_end;
			++(count);

			timer->expires_from_now(std::chrono::seconds(1));
			timer->async_wait(boost::bind(&TimerClass::print_forever,
										  this,
										  boost::asio::placeholders::error,
										  timer));
		}
		else if (boost::asio::error::operation_aborted == error.value())
		{
			std::cout << "timer canceled." << std::endl;
		}
	}

	bool start_forever_timer()
	{	
		boost::asio::io_service _io_service;

		count = 0;
		_forever_timer = new boost::asio::steady_timer(_io_service);
		_forever_timer->async_wait(boost::bind(&TimerClass::print_forever,
											   this,
											   boost::asio::placeholders::error,
											   _forever_timer));
		_io_service.run();

		//
		// _forever_timer 취소되면 _io_service.run() 메소드가 리턴함
		//
		log_info "start_forever_timer() returned" log_end;

		delete _forever_timer;  _forever_timer = nullptr;
		return true;
	}

	void stop_formever_timer()
	{
		if (nullptr == _forever_timer) return;
		_forever_timer->cancel();
		log_info "_forever_timer->cancel() called" log_end;
	}

};

bool test_boost_asio_timer_4()
{
	TimerClass tc;

	//==============
	//	유한 타이머
	//==============
	tc.start_timer();

		
	//==============
	//	무한 타이머를 실행하고, 
	//	3 초간 기다리다가
	//	외부에서 타이머를 중지
	//==============
	auto timer_task = Concurrency::create_task([&]()->void
	{
		tc.start_forever_timer();	
	});
	
	Sleep(1000 * 3);
	tc.stop_formever_timer();

	timer_task.wait();
	return true;
}