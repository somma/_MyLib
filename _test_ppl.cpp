/**
 * @file    _test_ppl.cpp
 * @brief   
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2017/02/22 09:51 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/

#include "stdafx.h"
#include <ppl.h>
#include <concurrent_queue.h>

#include <vector>
#include <queue>

#include "StopWatch.h"


bool test_for()
{
	std::vector<int> a1 = { 1,2,3,4,5 };
	std::vector<int> a2;
	std::vector<int> a3 = { 1,2,3,4,5 };
	for (int i=0; i<1024; ++i)
	{
		a2.push_back(i);
	}

	std::queue<std::vector<int> > q;
	q.push(a1);
	q.push(a2);
	q.push(a3);	

	for (int i = 0; i < 3; ++i)
	{		
		std::vector<int>& a = q.front();
		for (auto v : a)
		{
			log_info "tid=%u, %u",
				GetCurrentThreadId(),
				v
				log_end;
		}
		q.pop();
	}

	return true;

}

bool test_ppl_parallel_for()
{
	std::vector<int> a1 = { 1,2,3,4,5 };
	std::vector<int> a2;
	std::vector<int> a3 = { 1,2,3,4,5 };
	for (int i = 0; i < 32; ++i)
	{
		a2.push_back(i);
	}

	Concurrency::concurrent_queue<std::vector<int> > q;
	q.push(a1);
	q.push(a2);
	q.push(a3);

	Concurrency::parallel_for_each(
		q.unsafe_begin(),
		q.unsafe_end(),
		[&q](std::vector<int>& a)
		{			
			for (auto x : a)
			{
				log_info "tid=%u, %u",
					GetCurrentThreadId(),
					x
					log_end;
			}
			log_info "tid=%u, done",
				GetCurrentThreadId()
				log_end;
			
		});

	return true;

}


/// 
bool test_ppl()
{
	//test_for();
	test_ppl_parallel_for();

	return true;
}