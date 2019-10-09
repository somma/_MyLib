/**
 * @file    test_std_thread.cpp
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2019/10/09 09:51 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#include "stdafx.h"
#include "BaseWindowsHeader.h"

class ThreadParamClass
{
public:
	ThreadParamClass(int iv):_iv(iv)
	{
	}
	~ThreadParamClass()
	{
	}

	int _iv;
};

//#include <functional>
//using thread_ptr = std::unique_ptr<std::thread, void(*)(std::thread* t)>;
//thread_ptr make_thread_ptr(std::function func)
//{
//	thread_ptr p = new std::thread(func, [](std::thread* t) {
//		if (nullptr != t)
//		{
//			t->join();
//			delete t;
//		}
//	});
//	return p;
//}

bool test_std_thread_with_lambda()
{
	_mem_check_begin
	{
		//	Thread with
		//	- lambda
		//	- class parameter
		{
			ThreadParamClass p0(0);
			std::thread t0([](ThreadParamClass param) {
				log_info "Thread. tid=%u, param = %d",
					GetCurrentThreadId(),
					param._iv
					log_end;
			},
			p0);
			t0.join();
		}

		//	Thread pointer with
		//	- lambda
		//	- class parameter by value
		{
			ThreadParamClass p1(1);
			std::thread* t1 = new std::thread([](ThreadParamClass param) {
				log_info "Thread. tid=%u, param = %d",
					GetCurrentThreadId(),
					param._iv
					log_end;
			},
			p1);

			t1->join();
			delete t1;
		}


		// Thread pointer with 
		//	- lambda 
		//	- class parameter by ref
		{
			ThreadParamClass p2(2);
			std::thread* t2 = new std::thread([](ThreadParamClass& param) {
				log_info "Thread. tid=%u, param = %d",
					GetCurrentThreadId(),
					param._iv
					log_end;
			},
			std::ref(p2));

			t2->join();
			delete t2;
		}

		// Thread pointer with 
		//	- lambda 
		//	- class parameter by ref
		//	- unique_ptr
		{
			ThreadParamClass p3(3);
			auto t3_ptr = 
				std::make_unique<std::thread>([](ThreadParamClass& param) {
				log_info "Thread. tid=%u, param = %d",
					GetCurrentThreadId(),
					param._iv
					log_end;
			},
			std::ref(p3));
			t3_ptr->join();					//<!
		}

		// Thread pointer with 
		//	- lambda 
		//	- class parameter by ref
		//	- unique_ptr with custom delter that calls thread::join()
		{
			using thread_ptr = std::unique_ptr<std::thread, void(*)(std::thread* t)>;

			ThreadParamClass p3(3);
			thread_ptr t3_ptr = thread_ptr(
				new std::thread([](ThreadParamClass& param) 
				{
					log_info "Thread. tid=%u, param = %d",
						GetCurrentThreadId(),
						param._iv
						log_end;
				},std::ref(p3)), 
				[](std::thread* t) 
				{
					if (nullptr != t)
					{
						t->join();
						delete t;
					}			
			});
		}

		// Thread pointer with 
		//	- lambda 
		//	- class parameter by ref
		//	- unique_ptr with custom delter that calls thread::join()
		//
		//	쬐금 더 정리된 버전
		{
			using thread_ptr = std::unique_ptr<std::thread, void(*)(std::thread* t)>;
			auto thread_deleter = [](std::thread* p) 
			{
				if (nullptr != p)
				{
					p->join();
					delete p;
				}
			};

			ThreadParamClass p4(4);
			thread_ptr t3_ptr = thread_ptr(
				new std::thread([](ThreadParamClass& param)
				{
					log_info "Thread. tid=%u, param = %d",
						GetCurrentThreadId(),
						param._iv
						log_end;
				},
				std::ref(p4)),
				thread_deleter);
		}

		// Thread pointer with 
		//	- lambda 
		//	- class parameter by ref
		//	- unique_ptr with custom delter that calls thread::join()
		//
		//	쬐금 더~더~ 정리된 버전
		{
			using thread_ptr = std::unique_ptr<std::thread, void(*)(std::thread* t)>;
			auto make_thread_ptr = [](std::thread*&& t)->thread_ptr
			{
				return thread_ptr(t, [](std::thread* p)
				{
					if (nullptr != p)
					{
						p->join();
						delete p;
					}
				});
			};

			ThreadParamClass p5(5);
			thread_ptr t3_ptr = make_thread_ptr(
				new std::thread([](ThreadParamClass& param)
								{
									log_info "Thread. tid=%u, param = %d",
										GetCurrentThreadId(),
										param._iv
										log_end;
								},
								std::ref(p5)));
		}
	}
	_mem_check_end;


	return true;
}

using thread_ptr = std::unique_ptr<std::thread, void(*)(std::thread* t)>;
auto mtp = [](std::thread* t)->thread_ptr
{
	return thread_ptr(t, [](std::thread* p)
	{
		if (nullptr != p)
		{
			p->join();
			delete p;
		}
	});
};