/**
 * @file    _test_std_future_async.cpp
 * @brief
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    02/18/2023 Created.
 * @copyright (C)Somma, Inc. All rights reserved.
 **/
#include "stdafx.h"
#include <future>
#include <thread>
#include <chrono>
#include <queue>

///
///
///

void test_condition_variable()
{
	_mem_check_begin
	{
		std::mutex m;
		std::condition_variable cv;
		bool done = false;
		std::string info;

		std::thread t([&]() {
			{
				std::lock_guard<std::mutex> lk(m);
				info = "some data";
				done = true;
			}
			cv.notify_all();
			});

		std::unique_lock<std::mutex> lk(m);
		cv.wait(lk, [&] {return done; });
		lk.unlock();

		log_info "data=%s", info.c_str() log_end;
		t.join();
	}
	_mem_check_end;
}


///
///
///

void 
producer(
	_In_ std::queue<std::string>* pages, 
	_In_ std::mutex* lock, 
	_In_ int index,
	_In_ std::condition_variable* cv
)
{
	for (int i = 0; i < 5; ++i)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100 * index));
		std::string content =
			"index " + std::to_string(i) +
			" from thread " + std::to_string(index);

		lock->lock();
		pages->push(content);
		lock->unlock();

		// consumer 에게 content 가 추가되었음을 알린다. 
		cv->notify_one();
	}
}

void consumer(
	_In_ std::queue<std::string>* pages,
	_In_ std::mutex* lock,
	_In_ int* num_processed,
	_In_ std::condition_variable* cv
)
{
	while (*num_processed < 25)
	{
		std::unique_lock<std::mutex> lk(*lock);
		cv->wait(
			lk,
			[&](){
				// 조건이 거짓인 경우 unlock 하고, sleep 한다.
				//	-> true 를 리턴하면 깨어나고, 
				//	-> false 를 리턴하면 계속 sleep
				return !pages->empty() || *num_processed == 25;
			}
		);

		//
		// lock 된 상태로 깨어났음
		//

		if (*num_processed == 25)
		{
			lk.unlock();
			return;
		}

		std::string content = pages->front();
		pages->pop();

		++(*num_processed);
		lk.unlock();				//<!

		log_info
			"content: %s",
			content.c_str()
			log_end;
	}
}

void test_condition_variable_producer_consumer()
{
	_mem_check_begin
	{
		std::queue<std::string> pages;
		std::mutex pages_lock;
		std::condition_variable cv;

		std::vector<std::thread> producers;
		for (int i = 0; i < 5; ++i)
		{
			producers.push_back(
				std::thread(
					producer,
					&pages,
					&pages_lock,
					i + 1,
					&cv)
			);
		}

		int num_processed = 0;
		std::vector<std::thread> consumers;
		for (int i = 0; i < 3; ++i)
		{
			consumers.push_back(
				std::thread(
					consumer,
					&pages,
					&pages_lock,
					&num_processed,
					&cv)
			);
		}

		for (int i = 0; i < 5; ++i)
		{
			producers[i].join();
		}

		// consumer 들을 모두 깨운다. 
		cv.notify_all();

		for (int i = 0; i < 3; ++i)
		{
			consumers[i].join();
		}
	}
	_mem_check_end;
}

/// @brief	cv->notify_one() 호출갯수와 cv->wait() 이 깨어나는 수는 보장이 됨
///			notify_one 과 wait 이 깨어나는 횟수가 걱정되면 wait_for 쓰면 됨
void test_condition_variable_producer_consumer_2()
{
	_mem_check_begin
	{
		std::queue<std::string> pages;
		std::mutex pages_lock;
		std::condition_variable cv;

		int num_produced = 0;
		int num_consumed = 0;

		// producer
		auto p = std::thread([&]() {
			for (int i = 0; i < 1000; ++i)
			{
				pages_lock.lock();
				pages.push("content " + std::to_string(i));
				++num_produced;
				pages_lock.unlock();

				cv.notify_one();
			}
			});
			

		// consumer 		
		auto c = std::thread([&]() {
			while (num_consumed != num_produced)
			{
				std::unique_lock<std::mutex> lock(pages_lock);
				cv.wait(lock, [&]() {
					return !pages.empty();		// content 가 있으면 깨어남
					});

				std::string content = pages.front();
				pages.pop();
				++num_consumed;
				lock.unlock();				//<!

				log_info
					"content: %s",
					content.c_str()
					log_end;
			}
			});
		
		p.join();

		// consumer 들을 모두 깨운다. 
		cv.notify_all();
		c.join();

		_ASSERTE(num_produced == num_consumed);
	}
	_mem_check_end;
}

///
///
///

void worker(std::promise<std::string>* p)
{
	p->set_value("somma_data");
}

void test_future()
{
	_mem_check_begin
	{
		std::promise<std::string> p;
		std::future<std::string> data = p.get_future();

		std::thread t(worker, &p);
		data.wait();

		log_info "data=%s", data.get().c_str() log_end;

		t.join();
	} 
	_mem_check_end;
}

///
///
///

void test_future_exception()
{
	_mem_check_begin
	{
		std::promise<std::string> p;
		std::future<std::string> data = p.get_future();

		std::thread t([](std::promise<std::string>* p) {
			try
			{
				throw std::runtime_error("Some error");
			}
			catch (...)
			{
				p->set_exception(std::current_exception());
			}
			}, 
			&p);

		data.wait();

		try
		{
			log_info "data=%s", data.get().c_str() log_end;
		}
		catch (const std::exception& e)
		{
			log_err "exception: %s", e.what() log_end;
		}

		

		t.join();
	}
	_mem_check_end;
}

///
///
///

void test_future_wait_for()
{
	_mem_check_begin
	{
		std::promise<void> p;
		std::future<void> data = p.get_future();

		std::thread t([](std::promise<void>* p) {
			std::this_thread::sleep_for(std::chrono::seconds(10));
			p->set_value();
			}, 
			&p
		);

		while (true)
		{
			std::future_status status = data.wait_for(std::chrono::seconds(1));

			// 아직 준비 안되었음
			if (status == std::future_status::timeout)
			{
				log_dbg ">" log_end;
			}			

			// 준비 완료
			else if (status == std::future_status::ready)
			{
				log_dbg "" log_end;
				break;
			}
		}

		t.join();
	}
	_mem_check_end;
}



/// @brief	test suit
bool test_std_future_async()
{
	//test_condition_variable(); 
	//test_condition_variable_producer_consumer();
	test_condition_variable_producer_consumer_2();
	
	//test_future();
	//test_future_exception();
	//
	//test_future_wait_for();

	return true;
}