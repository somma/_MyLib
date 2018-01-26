/**
 * @file    
 * @brief   
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2018.01.25 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#include "stdafx.h"
#include <boost\thread.hpp>

#include "mm_queue.h"


typedef struct _dumy8
{
	int a;
	int b;
} dumy8;

bool test_mmque_one()
{
	uint32_t mmq_size = 32;

	//
	//	MMQ 持失
	//
	MmQueue mmq;
	if (true != mmq.initialize(mmq_size,
							   L"c:\\dbg\\mmq.dat"))
	{
		log_err "mmq.initialize() failed." log_end;
		return false;
	}

	//
	//	invalid requests
	//
	dumy8 d = { 1,2 };	
	_ASSERTE(true != mmq.push_back(1000, nullptr));
	_ASSERTE(true != mmq.push_back(1, nullptr));
	_ASSERTE(true != mmq.push_back(mmq_size + 1, (char*)&d));


	return true;
}

bool test_mmque_producer_consumer()
{
	volatile bool stop = false;

	//
	//	MMQ 持失
	//
	MmQueue mmq;
	if (true != mmq.initialize(32,
							   L"c:\\dbg\\mmq.dat"))
	{
		log_err "mmq.initialize() failed." log_end;
		return false;
	}

	//
	//	Producer thread 
	//
	boost::thread producer([&stop, &mmq]() {
		while (true != stop)
		{
			log_info "producer" log_end;
			Sleep(1000);

			dumy8 d = { 1,2 };
			mmq.push_back(sizeof(dumy8), (char*)&d);
		}

		log_info "Producer returned." log_end;
	});

	//
	//	Consumer thread
	//
	boost::thread consumer([&stop, &mmq]() {
		while (true != stop)
		{
			log_info "consumer" log_end;
			Sleep(1000);
			
			char* p = mmq.pop_front();
			if (nullptr != p)
			{
				dumy8* d = (dumy8*)p;
				log_info "a=%d, b=%d",
					d->a,
					d->b
					log_end;

				mmq.release(p);
			}
		}

		log_info "Consumer returned." log_end;
	});
	_pause;

	stop = true;
	producer.join();
	consumer.join();
	return true;
}


bool test_mmio_queue()
{
	if (!test_mmque_one()) return false;
//	if (!test_mmque_producer_consumer()) return false;

	return true;
}
